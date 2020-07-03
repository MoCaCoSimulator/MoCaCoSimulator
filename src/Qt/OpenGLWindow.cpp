#include "OpenGLWindow.h"
#include "../SetupScene.h"
#include "../EventManager.h"

OpenGLWindow::OpenGLWindow(QWidget* parent) : 
	QOpenGLWidget(parent),
	deltaTime(0.0),
	time(0.0),
	openGLFunctions(nullptr),
	scene(nullptr),
	initComplete(false),
	progressSliderMaxValue(10000)
{
	// Force widget update on frameswap
	connect(this, SIGNAL(frameSwapped()), this, SLOT(update()));
}

OpenGLWindow::~OpenGLWindow()
{
	scene->end();
	delete scene;
}

void OpenGLWindow::initializeGL()
{
#ifdef DEBUG_INPUT_INFORMATION
	qDebug("Init: OpenGLWindow");
#endif

	// Init input system state
	setFocus();
	setMouseTracking(true);

	// Init OpenGL state
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	// Grab OpenGL function set context format
	QOpenGLContext* context = this->context();
	openGLFunctions = context->functions();
	context->setFormat(this->format()); 
	//context->setShareContext(context);

	glewInit();

	// Start timer for computing deltatime
	timer.start();

	if (scene != nullptr)
	{
		scene->setSize(width(), height());
		scene->start();
	}

	initComplete = true;
}

void OpenGLWindow::resizeGL(int w, int h)
{
	scene->setSize(w, h);
}

MouseInput::MouseKey OpenGLWindow::QInputToCameraInput(Qt::MouseButton input)
{
	switch (input)
	{
	case Qt::MouseButton::LeftButton:
		return MouseInput::Left;
	case Qt::MouseButton::RightButton:
		return MouseInput::Right;
	case Qt::MouseButton::MiddleButton:
		return MouseInput::Middle;
	default:
		return MouseInput::None;
	}
}

void OpenGLWindow::paintGL()
{
#ifdef DEBUG_INPUT_INFORMATION
	qDebug("PaintGL");
#endif

	if (scene == nullptr)
		return;

	//compute deltaTime
	float now = timer.elapsed() * 0.001f;
	deltaTime = now - time;
	time = now;

	//qDebug() << time << "- ms:" << deltaTime << "FPS:" << (1.f / deltaTime);

	//clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Input events
	InputManager::HandleInputs();

	// Update the scene
	scene->update(deltaTime);
	scene->draw();
	
	// Check for OpenGL errors
	GLenum Error = glGetError();
	if (Error != 0)
		printf("opengl error: %d\n", Error);
	assert(Error == 0);
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent* event)
{
	QPoint mousePos = event->pos();

#ifdef DEBUG_INPUT_INFORMATION
	qDebug() << "Registered mouse movement at X: " << mousePos.x() << " Y: " << mousePos.y();
#endif

	InputManager::mousePosition = Vector2(mousePos.x(), mousePos.y());
}

void OpenGLWindow::mousePressEvent(QMouseEvent* event)
{
#ifdef DEBUG_INPUT_INFORMATION
		qDebug("Registered mouse press");
#endif

	Qt::MouseButton mouseButtonPressed = event->button();
	switch (mouseButtonPressed)
	{
	case Qt::LeftButton:
		InputManager::TriggerMouseButtonDownEvent(InputManager::Mousecode::Left);
		break;
	case Qt::RightButton:
		InputManager::TriggerMouseButtonDownEvent(InputManager::Mousecode::Right);
		break;
	case Qt::MidButton:
		InputManager::TriggerMouseButtonDownEvent(InputManager::Mousecode::Middle);
		break;
	default:
		std::cout << "Registered unknown mousebutton press with index " + mouseButtonPressed << std::endl;
		break;
	}
}

void OpenGLWindow::mouseReleaseEvent(QMouseEvent* event)
{
#ifdef DEBUG_INPUT_INFORMATION
	qDebug("Registered mouse release");
#endif

	Qt::MouseButton mouseButtonReleased = event->button();
	switch (mouseButtonReleased)
	{
	case Qt::LeftButton:
		InputManager::TriggerMouseButtonReleaseEvent(InputManager::Mousecode::Left);
		break;
	case Qt::RightButton:
		InputManager::TriggerMouseButtonReleaseEvent(InputManager::Mousecode::Right);
		break;
	case Qt::MidButton:
		InputManager::TriggerMouseButtonReleaseEvent(InputManager::Mousecode::Middle);
		break;
	default:
		std::cout << "Registered unknown mousebutton release with index " + mouseButtonReleased << std::endl;
		break;
	}
}

void OpenGLWindow::keyPressEvent(QKeyEvent* event)
{
#ifdef DEBUG_INPUT_INFORMATION
	qDebug("Registered key press");
#endif

	int keyIndex = event->key();
	switch (keyIndex)
	{
	case Qt::Key_Space:
		TriggerKeyDownEvent(InputManager::Keycode::Spacebar);
		break;
	case Qt::Key_Alt:
		TriggerKeyDownEvent(InputManager::Keycode::Alt);
		break;
	default:
		std::cout << "Action for keyindex " << keyIndex << " is not defined." << std::endl;
		break;
	}
}

void OpenGLWindow::keyReleaseEvent(QKeyEvent* event)
{
#ifdef DEBUG_INPUT_INFORMATION
	qDebug("Registered key release");
#endif

	int keyIndex = event->key();
	switch (keyIndex)
	{
	case Qt::Key_Space:
		TriggerKeyReleaseEvent(InputManager::Keycode::Spacebar);
		break;
	case Qt::Key_Alt:
		TriggerKeyReleaseEvent(InputManager::Keycode::Alt);
		break;
	default:
		std::cout << "Action for keyindex " << keyIndex << " is not defined." << std::endl;
		break;
	}
}

void OpenGLWindow::enterEvent(QEvent*)
{
	setFocus();
}

Scene* OpenGLWindow::GetCurrentScene()
{
	return scene;
}

void OpenGLWindow::RemoveCurrentScene()
{
	if(scene != nullptr)
		delete scene;
	scene = nullptr;
}

void OpenGLWindow::SetCurrentScene(Scene* scene)
{
	if(this->scene != nullptr)
		delete this->scene;
	this->scene = scene;
	if (initComplete)
	{
		makeCurrent();
		this->scene->setSize(width(), height());
		this->scene->start();
		doneCurrent();
	}
}

/*void OpenGLWindow::OnPlayButtonPressed()
{
	EventManager::instance().FireEvent("OnPlayButtonPressed");
}

void OpenGLWindow::OnPauseButtonPressed()
{
	EventManager::instance().FireEvent("OnPauseButtonPressed");
}*/

void OpenGLWindow::OnStopButtonPressed()
{
	EventManager::instance().FireEvent("OnStopButtonPressed");
}

void OpenGLWindow::OnTPoseButtonPressed()
{
	EventManager::instance().FireEvent("OnTPoseButtonPressed");
}

void OpenGLWindow::OnProgressSliderValueChanged(int value)
{
	float percentage = value / (float)progressSliderMaxValue;
	EventManager::instance().FireEvent("OnProgressSliderValueChanged", percentage);
}

void OpenGLWindow::OnTogglePlay()
{
	EventManager::instance().FireEvent("OnTogglePlay");
}

void OpenGLWindow::OnToggleSkeleton()
{
	EventManager::instance().FireEvent("OnToggleSkeleton");
}

void OpenGLWindow::OnToggleTrackerOffsets()
{
	EventManager::instance().FireEvent("OnToggleTrackerOffsets");
}
