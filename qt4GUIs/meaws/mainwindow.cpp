#include "mainwindow.h"

MainWindow::MainWindow() {
	marBackend=NULL;
	testingMethod=0;

	createMain();
	createActions();
	createMenus();
	createToolBars();
	readSettings();

	enableActions(1);
}

MainWindow::~MainWindow() {
	if (marBackend != NULL) {
		delete marBackend;
	}
}

void MainWindow::closeEvent(QCloseEvent *event) {
	if (1) {
	//if (maybeSave()) {
		writeSettings();
		event->accept();
	} else {
		event->ignore();
	}
}

void MainWindow::about() {
	QMessageBox::about(this, tr("About Meaws"),
		tr("Meaws (Musician Evaulation and Audition for Winds and Strings) "
		"is a learning tool for musicians.  It listens to a musician and "
		"displays the music with notes coloured based on their intonation."
		));
}

void MainWindow::readSettings() {
	QSettings settings("Metrosyas", "Metrosyas");
	QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
	QSize size = settings.value("size", QSize(700, 400)).toSize();
	resize(size);
	move(pos);
}

void MainWindow::writeSettings() {
	QSettings settings("Metrosyas", "Metrosyas");
	settings.setValue("pos", pos());
	settings.setValue("size", size());
}

void MainWindow::createActions() {
	newUserAct = new QAction(QIcon(":/images/new.png"), tr("&New user"), this);
	newUserAct->setShortcut(tr("Ctrl+N"));
	newUserAct->setStatusTip(tr("Create a new session"));
	connect(newUserAct, SIGNAL(triggered()), this, SLOT(newUser()));

	openAct = new QAction(QIcon(":/images/open.png"), tr("&Open user..."), this);
	openAct->setShortcut(tr("Ctrl+O"));
	openAct->setStatusTip(tr("Open an existing session"));
//	connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

	saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save user"), this);
	saveAct->setShortcut(tr("Ctrl+S"));
	saveAct->setStatusTip(tr("Save the session to disk"));
//	connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

	saveAsAct = new QAction(tr("Save user &As..."), this);
	saveAsAct->setStatusTip(tr("Save the session under a new name"));
//	connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

	closeAct = new QAction(QIcon(":/images/save.png"), tr("&Close user"), this);
	closeAct->setShortcut(tr("Ctrl+W"));
	closeAct->setStatusTip(tr("Close user"));
	connect(closeAct, SIGNAL(triggered()), this, SLOT(closeUser()));

	exitAct = new QAction(tr("E&xit"), this);
	exitAct->setShortcut(tr("Ctrl+Q"));
	exitAct->setStatusTip(tr("Exit the application"));
	connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

	aboutAct = new QAction(tr("&About Meaws"), this);
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

	aboutQtAct = new QAction(tr("About &Qt"), this);
	aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
	connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));


	openExerciseAct = new QAction(QIcon(":/images/open.png"), tr("Open &Exercise..."), this);
	openExerciseAct->setShortcut(tr("Ctrl+R"));
	openExerciseAct->setStatusTip(tr("Open a new exercise"));
	connect(openExerciseAct, SIGNAL(triggered()), this, SLOT(openExercise()));

	startMetroAct = new QAction(QIcon(":/images/player_play.png"), tr("Start..."), this);

	stopMetroAct = new QAction(QIcon(":/images/player_stop.png"), tr("Stop..."), this);
}

// main window area
void MainWindow::createMain() {
	QFrame* centralFrame = new QFrame;
	setCentralWidget(centralFrame);

	// this is what displays the picture of the exercise.
	imageLabel = new QLabel;
	imageLabel->setBackgroundRole(QPalette::Base);
	imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	imageLabel->setScaledContents(false);

	// this is what displays our testing text.  Later on we would
	// remove textLable and make a QT painting area or make it a picture.
	textLabel = new QLabel;
	updateTestingMethod();

	// we want to display the above two QLabels within our main window.
	mainLayout = new QVBoxLayout;
	mainLayout->addWidget(imageLabel,0,Qt::AlignTop);
	mainLayout->addWidget(textLabel);
	centralFrame->setLayout(mainLayout);
}

void MainWindow::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(newUserAct);
	fileMenu->addAction(openAct);
	fileMenu->addAction(saveAct);
	fileMenu->addAction(saveAsAct);
	fileMenu->addSeparator();
	fileMenu->addAction(closeAct);
	fileMenu->addAction(exitAct);

	// exercise menu
	exerMenu = menuBar()->addMenu(tr("Exercise"));
	exerMenu->addAction(openExerciseAct);
	exerMenu->setEnabled(false);

	menuBar()->addSeparator();
	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(aboutAct);
	helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars() {
	userToolBar = addToolBar(tr("User"));
	userToolBar->addAction(newUserAct);
	userToolBar->addAction(openAct);
	userToolBar->addAction(saveAct);
	userToolBar->addAction(closeAct);
	userToolBar->addAction(openExerciseAct);

	tempoToolBar = addToolBar(tr("Tempo"));
	tempoBox = new QSpinBox();
	tempoBox->setRange(30,240);
	tempoBox->setValue(60);
	tempoToolBar->addWidget(tempoBox);

	slider = new QSlider(Qt::Horizontal);
	slider->setRange(30, 240);
	slider->setValue(60);
//		slider->setMinimumWidth(60);
	tempoToolBar->addWidget(slider);

	tempoToolBar->addAction(startMetroAct);
	tempoToolBar->addAction(stopMetroAct);

	connect(slider, SIGNAL(valueChanged(int)),
		tempoBox, SLOT(setValue(int)));
	connect(tempoBox, SIGNAL(valueChanged(int)),
		slider, SLOT(setValue(int)));

	infoBar = addToolBar(tr("Info"));
	exerciseTitle = new QLabel();
	exerciseTitle->setText("");
	infoBar->addWidget(exerciseTitle);

	QLabel *userNameLabel = new QLabel();
	userNameLabel->setText(userName);
	infoBar->addWidget(userNameLabel);
}

void MainWindow::openExercise() {
	QString exerciseName = QFileDialog::getOpenFileName(this,
		tr("Open Exercise"),"exercises/",tr("Exercises (*.png)"));
	if (!exerciseName.isEmpty()) {
		QImage image(exerciseName);
		imageLabel->setPixmap(QPixmap::fromImage(image));
		
		exerciseTitle->setText( tr("Exercise: %1").arg(QFileInfo(exerciseName).baseName()) );
		enableActions(3);
	}
}

bool MainWindow::maybeTestingMethod() {
	if (testingMethod>0) {
		return true;
	} else {
		return chooseTestingMethod();
	}
}

bool MainWindow::chooseTestingMethod() {
	QStringList items;
	items << tr("String intonation test") << tr("Wind air control test");
	bool ok;
	QString item = QInputDialog::getItem(this, tr("Choose testing method"),
		tr("TestingMethod:"), items, 0, false, &ok);
	if (ok && !item.isEmpty()) {
		if (item=="String intonation test") testingMethod=1;
		if (item=="Wind air control test") testingMethod=2;
		updateTestingMethod();
		return true;
	} else {
		return false;
	}
}

void MainWindow::updateTestingMethod() {
	if (testingMethod==0) textLabel->setText("No testing method selected");
	if (testingMethod==1) {
		textLabel->setText("String intonation test");
	}
	if (testingMethod==2) {
		textLabel->setText("Wind air control test");
	}
}

bool MainWindow::chooseUserInfo() {
	bool ok;
	QString text = QInputDialog::getText(this, tr("User info"),
		tr("User name:"), QLineEdit::Normal,
		QDir::home().dirName(), &ok);
	if (ok && !text.isEmpty()) {
		userName = text;
		return true;
	} else {
		return false;
	}
}



void MainWindow::newUser() {
	if (chooseUserInfo()) {
		if (chooseTestingMethod()) {  // force a choice here; no `maybe'
			enableActions(2);
		}
	}
}

void MainWindow::enableActions(int state) {
	if (state==1) {
		setWindowTitle(tr("Meaws"));

		saveAct   ->setEnabled(false);
		saveAsAct ->setEnabled(false);
		closeAct  ->setEnabled(false);

		infoBar     ->setEnabled(false);
		exerMenu    ->setEnabled(false);
		openExerciseAct ->setEnabled(false);

		tempoToolBar ->setEnabled(false);
	}
	if (state==2) {
		setWindowTitle(tr("Meaws - %1").arg(userName));

		saveAct   ->setEnabled(true);
		saveAsAct ->setEnabled(true);
		closeAct  ->setEnabled(true);

		infoBar     ->setEnabled(true);
		exerMenu    ->setEnabled(true);
		openExerciseAct ->setEnabled(true);

		tempoToolBar ->setEnabled(false);
	}
	if (state==3) {
		setupMarBackend();
		tempoToolBar ->setEnabled(true);

	}
}

void MainWindow::setupMarBackend() {
	if (marBackend != NULL) {
		delete marBackend;
		marBackend = NULL;
	}
	marBackend = new MarBackend(testingMethod);

	// communication with Marsyas backend
	connect(startMetroAct, SIGNAL(triggered()), marBackend, SLOT(startMetro()));
	connect(stopMetroAct, SIGNAL(triggered()), marBackend, SLOT(stopMetro()));

	connect(slider, SIGNAL(valueChanged(int)),
		marBackend, SLOT(setTempo(int)));
	connect(tempoBox, SIGNAL(valueChanged(int)),
		marBackend, SLOT(setTempo(int)));

}

void MainWindow::closeUser() {
	if (marBackend != NULL) {
		delete marBackend;
		marBackend = NULL;
	}

	imageLabel->clear();
	userName="No user";
	testingMethod=0;
	updateTestingMethod();
	enableActions(1);
}


