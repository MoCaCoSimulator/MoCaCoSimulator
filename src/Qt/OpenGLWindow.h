#include <QApplication>
#include "../scene.h"
#include <QOpenGLWindow>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>
#include <QSurfaceFormat>
#include <QtOpenGL>
#include "../MouseInput.h"
#include <QDebug>
#include "../InputManager.h"

#pragma once

// Uncomment this if input debugging informations are needed
//#define DEBUG_INPUT_INFORMATION

class OpenGLWindow : public QOpenGLWidget, public InputManager
{
	Q_OBJECT
public slots:
	//void OnPlayButtonPressed();
	//void OnPauseButtonPressed();
	void OnStopButtonPressed();
	void OnTPoseButtonPressed();
	void OnProgressSliderValueChanged(int value);
	void OnTogglePlay();
	void OnToggleSkeleton();
	void OnToggleTrackerOffsets();
public:
	OpenGLWindow(QWidget* parent);
	~OpenGLWindow();

	Scene* GetCurrentScene();
	void RemoveCurrentScene();
	void SetCurrentScene(Scene* scene);
protected:
	virtual void initializeGL() override;
	virtual void resizeGL(int w, int h) override;
	virtual void paintGL() override;
	
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;
	void enterEvent(QEvent*) override;

	MouseInput::MouseKey OpenGLWindow::QInputToCameraInput(Qt::MouseButton input);
private:
	bool initComplete;
	int progressSliderMaxValue;

	QOpenGLFunctions* openGLFunctions;
	QOpenGLShaderProgram program;
	QElapsedTimer timer;

	float deltaTime;
	float time;

	Scene* scene;
};