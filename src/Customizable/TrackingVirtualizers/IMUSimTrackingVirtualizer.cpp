#include "IMUSimTrackingVirtualizer.h"

RegisterVirtualizer<IMUSimTrackingVirtualizer> IMUSimTrackingVirtualizer::Register;

bool IMUSimTrackingVirtualizer::pythonEnvironmentActive = false;
PyObject* IMUSimTrackingVirtualizer::pFunc = nullptr;
int IMUSimTrackingVirtualizer::referenceCounter = 0;

IMUSimTrackingVirtualizer::IMUSimTrackingVirtualizer() : BaseTrackingVirtualizer("IMUSimTrackingVirtualizer")
{
	// Setup the python environment
	if (!pythonEnvironmentActive)
	{
		qDebug() << "Initializing python";

		Py_Initialize();

		// Set the python up for
		PyRun_SimpleString("import sys");
		PyRun_SimpleString("import os");
		PyRun_SimpleString("sys.path.append(os.getcwd() + '/src/Python')");

		pythonEnvironmentActive = true;
	}

	referenceCounter++;

	AddParameter(new Parameter<int>("Input Sampling Rate", 60));
	AddParameter(new Parameter<int>("Laser Sweep Sampling Rate", 120));
	AddParameter(new Parameter<int>("IMU Update Rate", 366));
	AddParameter(new Parameter("IMU Model", CustomEnums::IMUModel::Ideal));
	AddParameter(new Parameter("Orientation Filter", CustomEnums::OrientationFilter::GyroIntegrator));
	AddParameter(new Parameter("Calibrate", false));
}

IMUSimTrackingVirtualizer::~IMUSimTrackingVirtualizer()
{
	referenceCounter--;

	// Destroy the python environment if not longer needed
	if (pythonEnvironmentActive && referenceCounter == 0)
	{
		qDebug() << "Destroying python";

		Py_Finalize();
		pythonEnvironmentActive = false;
	}
}

bool IMUSimTrackingVirtualizer::CreateOutputAnimation(TrackerHandle& trackerHandle, AnimationCurve& output)
{
	float animationLength = trackerHandle.GetAnimationLength();
	int inputSamplingRate = dynamic_cast<Parameter<int>*>(parameters["Input Sampling Rate"])->GetValue();
	int frameCount = inputSamplingRate * animationLength;

	// IMUSim cant work with less than five keyframes
	if (frameCount < 5)
		return false;

	// Create a python array with the timestamps
	PyObject* timestamps = PyList_New(frameCount);
	// Create a python array with the positions
	PyObject* positions = PyList_New(3);
	PyObject* xPositions = PyList_New(frameCount);
	PyObject* yPositions = PyList_New(frameCount);
	PyObject* zPositions = PyList_New(frameCount);
	// Create a python array with the rotations
	PyObject* rotations = PyList_New(4);
	PyObject* wRotations = PyList_New(frameCount);
	PyObject* xRotations = PyList_New(frameCount);
	PyObject* yRotations = PyList_New(frameCount);
	PyObject* zRotations = PyList_New(frameCount);
	for (size_t i = 0; i < frameCount; ++i)
	{
		float normalizedTime = i / (float)frameCount;
		float time = normalizedTime * animationLength;

		PyList_SetItem(timestamps, i, PyFloat_FromDouble(time));

		Vector3 position = trackerHandle.GetPosition(normalizedTime);

		PyList_SetItem(xPositions, i, PyFloat_FromDouble(position.x));
		PyList_SetItem(yPositions, i, PyFloat_FromDouble(position.y));
		PyList_SetItem(zPositions, i, PyFloat_FromDouble(position.z));

		Quaternion rotation = trackerHandle.GetRotation(normalizedTime);
		rotation = rotation.normalized();

		PyList_SetItem(wRotations, i, PyFloat_FromDouble(rotation.w));
		PyList_SetItem(xRotations, i, PyFloat_FromDouble(rotation.x));
		PyList_SetItem(yRotations, i, PyFloat_FromDouble(rotation.y));
		PyList_SetItem(zRotations, i, PyFloat_FromDouble(rotation.z));
	}
	PyList_SetItem(positions, 0, xPositions);
	PyList_SetItem(positions, 1, zPositions);
	PyList_SetItem(positions, 2, yPositions); // Z is Y in IMUSim

	PyList_SetItem(rotations, 0, wRotations);
	PyList_SetItem(rotations, 1, xRotations);
	PyList_SetItem(rotations, 2, yRotations);
	PyList_SetItem(rotations, 3, zRotations);

	CustomEnums::IMUModel imuModel = dynamic_cast<Parameter<CustomEnums::IMUModel>*>(parameters["IMU Model"])->GetValue();
	CustomEnums::OrientationFilter orientationFilter = dynamic_cast<Parameter<CustomEnums::OrientationFilter>*>(parameters["Orientation Filter"])->GetValue();
	bool calibrate = dynamic_cast<Parameter<bool>*>(parameters["Calibrate"])->GetValue();
	int imuFramerate = dynamic_cast<Parameter<int>*>(parameters["IMU Update Rate"])->GetValue();
	int viveFramerate = dynamic_cast<Parameter<int>*>(parameters["Laser Sweep Sampling Rate"])->GetValue();

	// Setup the argument list
	// 1. IMU Framerate
	// 2. Timestamps
	// 3. Positions
	// 4. Rotations
	// 5. IMU model id
	// 6. Orientation filter id
	// 7. Calibration flag
	PyObject* arglist = PyTuple_New(7);
	PyTuple_SetItem(arglist, 0, PyFloat_FromDouble(1.0 / imuFramerate));
	PyTuple_SetItem(arglist, 1, timestamps);
	PyTuple_SetItem(arglist, 2, positions);
	PyTuple_SetItem(arglist, 3, rotations);
	PyTuple_SetItem(arglist, 4, PyLong_FromLong((int)imuModel));
	PyTuple_SetItem(arglist, 5, PyLong_FromLong((int)orientationFilter));
	PyTuple_SetItem(arglist, 6, PyBool_FromLong(calibrate));

	PyObject* pResult = nullptr;

	PyObject* pName = PyUnicode_FromString("IMUSimScript2");
	PyObject* pModule = PyImport_Import(pName);
	Py_DECREF(pName);
	if (pModule != nullptr)
	{
		pFunc = PyObject_GetAttrString(pModule, "calculate_trajectory");
		
	}
	else
	{
		qDebug() << "Could not load module";
		return false;
	}

	if (pFunc && PyCallable_Check(pFunc))
	{
		pResult = PyObject_CallObject(pFunc, arglist);
		Py_DECREF(arglist);
		if (pResult == NULL)
		{
			Py_DECREF(pFunc);
			qDebug() << "Call failed";
			return false;
		}

		// create a new interpreter 
		//PyThreadState* pThreadState = Py_NewInterpreter();
		//PyThreadState_Swap(pThreadState);
		//assert(pThreadState != NULL);
		//// execute some Python
		//pResult = PyObject_CallObject(pFunc, arglist);
		//Py_DECREF(arglist);
		//if (pResult == NULL)
		//{
		//	Py_DECREF(pFunc);
		//	//Py_DECREF(pModule);
		//	PyErr_Print();
		//	fprintf(stderr, "Call failed\n");
		//}

		//Py_EndInterpreter(pThreadState);
	}
	else
	{
		qDebug() << "Function is not callable";
		return false;
	}

	/*Py_DECREF(rotations);
	Py_DECREF(wRotations);
	Py_DECREF(xRotations);
	Py_DECREF(yRotations);
	Py_DECREF(zRotations);
	Py_DECREF(positions);
	Py_DECREF(xPositions);
	Py_DECREF(yPositions);
	Py_DECREF(zPositions);
	Py_DECREF(timestamps);*/

	if (pResult == nullptr)
	{
		qDebug() << "The call failed";
		PyErr_Print();
	}
	else if (pResult == Py_None)
	{
		qDebug() << "The call returned none";
		PyErr_Print();
	}
	else
	{
		qDebug() << "The call was a success";

		std::vector<float> sampleTimes;

		// Split the result
		PyObject* pTimestampObject = nullptr;
		PyObject* pRotationObject = nullptr;
		PyObject* pPositionObject = nullptr;

		if (!PyArg_ParseTuple(pResult, "OOO", &pTimestampObject, &pRotationObject, &pPositionObject))
		{
			qDebug() << "Parsing return values failed";
			return false;
		}

		if (pTimestampObject == nullptr)
		{
			qDebug() << "Timestamp object is null";
			return false;
		}

		if (pRotationObject == nullptr)
		{
			qDebug() << "Rotation object is null";
			return false;
		}

		if (pPositionObject == nullptr)
		{
			qDebug() << "Position object is null";
			return false;
		}

		std::vector<AnimationCurve::VectorAnimationKey> estimatedPositionCurve;
		std::vector<AnimationCurve::QuaternionAnimationKey> estimatedRotationCurve;

		int timeStampCount = PyList_Size(pTimestampObject);
		for (size_t i = 0; i < timeStampCount; i++)
		{

			float timeStamp = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(pTimestampObject, i)));
			sampleTimes.push_back(timeStamp);

			float sampleTime = sampleTimes[i];

			PyObject* pVector = PyList_GetItem(pPositionObject, i);
			float posX = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(pVector, 0)));
			float posZ = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(pVector, 1)));
			float posY = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(pVector, 2))); // Z is Y in IMUSim
			Vector3 estimatedPosition = Vector3(posX, posY, posZ);
			estimatedPositionCurve.push_back(AnimationCurve::VectorAnimationKey(sampleTime, estimatedPosition));

			PyObject* pQuaternion = PyList_GetItem(pRotationObject, i);
			float rotX = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(pQuaternion, 0)));
			float rotY = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(pQuaternion, 1)));
			float rotZ = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(pQuaternion, 2)));
			float rotW = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(pQuaternion, 3)));
			Quaternion estimatedRotation = Quaternion(rotX, rotY, rotZ, rotW);
			estimatedRotationCurve.push_back(AnimationCurve::QuaternionAnimationKey(sampleTime, estimatedRotation));
		}

		Py_DECREF(pResult);
		Py_XDECREF(pFunc);
		Py_DECREF(pModule);
		//Py_DECREF(pRotationObject);
		//Py_DECREF(pPositionObject);

		std::vector<float> viveSweeps;
		std::vector<Vector3> positionOffsets;
		std::vector<Quaternion> rotationOffsets;
		float currentTime = estimatedPositionCurve[0].time;
		while (currentTime < estimatedPositionCurve[estimatedPositionCurve.size() - 1].time)
		{
			Quaternion realRotation = trackerHandle.GetRotation(currentTime / animationLength);
			Vector3 realPosition = trackerHandle.GetPosition(currentTime / animationLength);

			Vector3 estimatedPosition = AnimationCurve::VectorAnimationKey::Interpolate(currentTime, estimatedPositionCurve);
			Quaternion estimatedRotation = AnimationCurve::QuaternionAnimationKey::Interpolate(currentTime, estimatedRotationCurve);

			Vector3 positionOffset = realPosition - estimatedPosition;
			Quaternion rotationOffset = realRotation * Quaternion::Inverse(estimatedRotation);

			//qDebug() << "Pos Offset: " << positionOffset.toString().c_str();
			//qDebug() << "Rot Offset: " << rotationOffset.toString().c_str();

			positionOffsets.push_back(positionOffset);
			rotationOffsets.push_back(rotationOffset);

			viveSweeps.push_back(currentTime);

			currentTime += 1.0f / (float)viveFramerate;
		}

		int internalFrameCount = trackerHandle.GetAnimationLength() * imuFramerate;
		for (size_t i = 0; i < internalFrameCount; i++)
		{
			float normalizedTime = i / (float)internalFrameCount;
			float frameTime = trackerHandle.GetAnimationLength() * normalizedTime;

			bool hasBefore = false;
			bool hasAfter = false;
			for (size_t i = 0; i < sampleTimes.size(); i++)
			{
				float sampleTime = sampleTimes[i];
				if (sampleTime <= frameTime)
				{
					hasBefore = true;
				}
				if (sampleTime >= frameTime)
				{
					hasAfter = true;
				}
				if (hasBefore && hasAfter)
					break;
			}

			Quaternion realRotation = trackerHandle.GetRotation(normalizedTime);
			Vector3 realPosition = trackerHandle.GetPosition(normalizedTime);

			// IMU Sim has entry
			if (hasBefore && hasAfter)
			{
				Vector3 position = AnimationCurve::VectorAnimationKey::Interpolate(frameTime, estimatedPositionCurve);
				Quaternion rotation = AnimationCurve::QuaternionAnimationKey::Interpolate(frameTime, estimatedRotationCurve);

				int timePeriod;
				for (size_t i = 0; i < viveSweeps.size(); i++)
				{
					float currentSweep = viveSweeps[i];
					if (i == viveSweeps.size() - 1)
					{
						timePeriod = i;
						break;
					}
					float nextSweep = viveSweeps[i + 1];
					if (frameTime >= currentSweep && frameTime <= nextSweep)
					{
						timePeriod = i;
						break;
					}
				}

				Vector3 offsetPosition = positionOffsets[timePeriod];
				Quaternion offsetRotation = rotationOffsets[timePeriod];

				Vector3 resultPosition = position + offsetPosition;
				Quaternion resultRotation = rotation * offsetRotation;

				output.positions.push_back(AnimationCurve::VectorAnimationKey(frameTime, resultPosition));
				output.rotations.push_back(AnimationCurve::QuaternionAnimationKey(frameTime, resultRotation));
			}
			// No time at the end				No time in the front
			else if ((hasBefore && !hasAfter) || (!hasBefore && hasAfter))
			{
				output.positions.push_back(AnimationCurve::VectorAnimationKey(frameTime, realPosition));
				output.rotations.push_back(AnimationCurve::QuaternionAnimationKey(frameTime, realRotation));
			}
		}

		output.scalings.push_back(AnimationCurve::VectorAnimationKey(0.0f, Vector3::one));
		output.scalings.push_back(AnimationCurve::VectorAnimationKey(animationLength, Vector3::one));

		return true;
	}
}

BaseTrackingVirtualizer* IMUSimTrackingVirtualizer::Clone() const
{
	return new IMUSimTrackingVirtualizer();
}
