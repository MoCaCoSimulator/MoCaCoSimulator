from imusim.all import *
import imusim.maths.quaternions
import numpy as np
import traceback
import gc
from importlib import reload

def calculate_trajectory(samplingPeriod, laserSweepPeriod, timestamps, positions, rotations, imu_model_id, gyro_filter_id, calibrate):
    try:
        reload(imusim.maths.quaternions)

        # Open a data log to get the python outputs
        data = open("data.log", "w+")

        # Convert data from normal python to numpy or IMUSim's QuaternionArray
        np_timestamps = np.asarray(timestamps)
        np_positions = np.asarray(positions)
        np_rotations = QuaternionArray(rotations)
        #np_rotations = randomRotationSequence(np_timestamps)
        # np_rotations = QuaternionArray(rotations)
    #        quat_list = []
    #        for index in range(len(rotations[0])):
    #            w = rotations[0][index]
    #            x = rotations[1][index]
    #            y = rotations[2][index]
    #            z = rotations[3][index]
    #        quat = Quaternion(w,x,y,z)
        #    quat_list.append(quat)
        #np_rotations = QuaternionArray(quat_list)

        # data.write("Len: " + str(len(np_rotations)))

        # Create time series of the positions and rotations
        pos_time_series = TimeSeries(np_timestamps, np_positions)
        rot_time_series = TimeSeries(np_timestamps, np_rotations)

        # Create a sampled trajectory from the time series
        sampled_trajectory = SampledTrajectory(pos_time_series, rot_time_series)

        # Convert the sampled time series into a splined one
        splined_trajectory = SplinedTrajectory(sampled_trajectory)
        # splined_trajectory = RandomTrajectory()

        t = randomTimeSequence()
        p = randomPositionSequence(t)
        r = randomRotationSequence(t)

        data.write(str(t))

        # Calculate the times of the laser sweeps
        current_time = splined_trajectory.startTime
        time_periods = []
        time_periods.append(splined_trajectory.startTime)
        while current_time + laserSweepPeriod < splined_trajectory.endTime:
            current_time += laserSweepPeriod
            time_periods.append(current_time)
        time_periods[len(time_periods) - 1] = splined_trajectory.endTime
        # data.write("There are " + str(len(time_periods) - 1) + " time periods\n")

        # Create lists for complete results
        complete_timestamps = []
        complete_positions = []
        complete_rotations = []

        for i in range(len(time_periods) - 1):
            data.write("Calculating time period " + str(i + 1) + "/" + str(len(time_periods)) + "\n")

            time_period_start_time = time_periods[i]
            time_period_end_time = time_periods[i + 1]

            data.write("After time period times\n")

            # Create a simulation
            env = Environment()
            sim = Simulation(environment=env)

            data.write("After env and sim\n")

            # Create an IMU
            if(imu_model_id == 0):
                data.write("Using Ideal IMU\n")
                imu = IdealIMU()
            elif(imu_model_id == 1):
                data.write("Using Orient3 IMU\n")
                imu = Orient3IMU()
            else:
                data.write("Other IMU models have not been implemented yet.\n")

            # Setup the gyroscope integrator
            if(gyro_filter_id == 0):
                data.write("Using gyro integrator\n")
                filter = GyroIntegrator(time_period_start_time, splined_trajectory.rotation(time_period_start_time))
            elif(gyro_filter_id == 1):
                data.write("Using OrientCF\n")
                filter = OrientCF(time_period_start_time, splined_trajectory.rotation(time_period_start_time), 1, 1)
            elif(gyro_filter_id == 2):
                data.write("Using YunEKF\n")
                filter = YunEKF(time_period_start_time,
                                splined_trajectory.rotation(time_period_start_time),
                                initialRotationalVelocity = np.zeros((3,1)),
                                initialCovariance=np.asmatrix(np.diag([0.01]*3 + [0.0001]*4)),
                                measurementCovariance=np.asmatrix(np.diag([0.01]*3 + [0.0001]*4)),
                                D=50,
                                tau=0.5)
            elif(gyro_filter_id == 3):
                data.write("Using BachmannCF\n")
                filter = BachmannCF(time_period_start_time, splined_trajectory.rotation(time_period_start_time), 1)
            else:
                data.write("Other orientation filters have not been implemented yet.\n")

            # Calibrate the IMU based on the environment
            if(calibrate):
                data.write("Calibrating IMU\n")
                samples = 1000
                rotationalVelocity = 20
                calibrator = ScaleAndOffsetCalibrator(env, samples, samplingPeriod, rotationalVelocity)
                calibration = calibrator.calibrate(imu)

            imu.simulation = sim
            imu.trajectory = splined_trajectory

            data.write("Before behaviour\n")

            # Create a behaviour for the IMU
            if(calibrate):
                behaviour = BasicIMUBehaviour(imu, samplingPeriod, initialTime=time_period_start_time, filter=filter, calibration=calibration)
            else:
                behaviour = BasicIMUBehaviour(imu, samplingPeriod, initialTime=time_period_start_time, filter=filter)

            data.write("Before simulation\n")

            # Setup the start time of the simulation
            sim.time = time_period_start_time
            # Run the simulation
            sim.run(time_period_end_time)

            data.write("After simulation\n")

            # Convert IMUSim's quaternion type back to python array
            primitiveEstimatedRotations = [[0.0,0.0,0.0,0.0] for i in range(len(filter.rotation))]
            for i in range(len(filter.rotation)):
                primitiveEstimatedRotations[i] = [filter.rotation.values[i].x, filter.rotation.values[i].y, filter.rotation.values[i].z, filter.rotation.values[i].w]

            complete_rotations += primitiveEstimatedRotations

            data.write("After gyro integration\n")

            # Calculate the positions based on acceleration
            if(calibrate):
                accel_measurements = imu.accelerometer.calibratedMeasurements.values
                accel_timestamps = imu.accelerometer.calibratedMeasurements.timestamps
            else:
                accel_measurements = imu.accelerometer.rawMeasurements.values
                accel_timestamps = imu.accelerometer.rawMeasurements.timestamps

            data.write("After accel calibrate\n")

            data.write("Splined Starttime: " + str(splined_trajectory.startTime) + "\n")
            data.write("Timestamps Starttime: " + str(accel_timestamps[0]) + "\n")
            initialPosition = splined_trajectory.position(accel_timestamps[0])
            initialVelocity = splined_trajectory.velocity(accel_timestamps[0])

            data.write("After initial data\n")

            #data.write("Initial Position: " + str(initialPosition) + "\n")
            #data.write("Initial Velocity: " + str(initialVelocity) + "\n")
            integrationMethod = integrators.RectangleRule # Options: integrators.TrapeziumRule / integrators.RectangleRule

            data.write("After integration method\n")

            estimatedPositions = [[0.0,0.0,0.0] for i in range(len(accel_timestamps))]
            estimatedPositions[0] = [initialPosition[0], initialPosition[1], initialPosition[2]]

            lastPosition = initialPosition
            lastVelocity = initialVelocity
            currentPosition = initialPosition
            currentVelocity = initialVelocity
            counter = 1

            data.write("Before position integration\n")

            position_integrator = integrators.DoubleIntegrator(initialPosition, initialVelocity, integrationMethod)
            for accel_index in range(accel_measurements.shape[1] - 1):
                time = accel_timestamps[accel_index]

                accel = accel_measurements[:,accel_index:accel_index+1]
                #accel = splined_trajectory.acceleration(time)

                # estimated_rotation = filter.rotation.values[accel_index]
                estimated_rotation = splined_trajectory.rotation(time)

                #data.write("Accel Val: " + str(accel) + "\n")

                # Rotate the acceleration to global space: https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
                rot_vector = np.array([estimated_rotation.x, estimated_rotation.y, estimated_rotation.z])
                accel_vector = np.array([accel[0][0], accel[1][0], accel[2][0]])
                s = estimated_rotation.w
                cross_rot_accel = np.cross(rot_vector, accel_vector)
                dot_rot_accel = np.dot(rot_vector, accel_vector)
                dot_rot_rot = np.dot(rot_vector, rot_vector)
                vec1 = 2.0 * dot_rot_accel * rot_vector
                vec2 = (s * s - dot_rot_rot) * accel_vector
                vec3 = 2.0 * s * cross_rot_accel
                accel_rotated = vec1 + vec2 + vec3
                accel[0][0] = accel_rotated[0]
                accel[1][0] = accel_rotated[1]
                accel[2][0] = accel_rotated[2] # + sim.environment.gravitationalField.nominalValue[2]
                #data.write("Accel Rot Val: " + str(accel) + "\n")

                # Estimation version 1
                position = position_integrator(accel, samplingPeriod)
                estimatedPositions[counter] = [position[0], position[1], position[2]]

                # Estimation version 2
                velocity_integrator = integrators.RectangleRule(lastVelocity)
                position_integrator = integrators.RectangleRule(lastPosition)
                new_velocity = velocity_integrator(accel, samplingPeriod)
                new_position = position_integrator(new_velocity, samplingPeriod)
                lastPosition = new_position
                lastVelocity = new_velocity

                # Estimation version 3
                currentPosition += currentVelocity * samplingPeriod
                currentVelocity += accel * samplingPeriod

                #data.write("One-By-One Estimation: " + str(new_position) + "\n")
                #data.write("Iterative Estimation: " + str(currentPosition) + "\n")
                #data.write("Double Integration Estimation: " + estimatedPositions[counter].__str__() + "\n")
                #data.write("Real Position: " + str(splined_trajectory.position(time)) + "\n")

                counter += 1

            data.write("After position integration\n")

            complete_positions += estimatedPositions

            list_accel_timestamps = list(accel_timestamps)
            complete_timestamps += list_accel_timestamps

            data.write("End iteration\n")

            # gc.collect()

        # data.write("Timestamps: " + str(complete_timestamps))
        # data.write("Positions: "  + str(complete_positions))
        # data.write("Rotations: "  + str(complete_rotations))

        logf = open("error.log", "w+")
        logf.write("No error occured")

        return complete_timestamps, complete_rotations, complete_positions
    except Exception as e:
        print("Error occured")
        print(str(e))
        print(traceback.format_exc())
        logf = open("error.log", "w+")
        logf.write(str(e))
        logf.write(traceback.format_exc())
        return None
