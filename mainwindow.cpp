#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QPixmap>
#include <QStandardPaths>
#include <QFileDialog>
#include <QSettings>
#include <QDir>
#include <QFileInfoList>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentVideoIndex(0), currentImageIndex(0)
{
    // Set minimum size for the window
    setMinimumSize(800, 400);

    // Create QLabel as the central widget
    displayLabel = new QLabel(this);
    displayLabel->setAlignment(Qt::AlignCenter);
    displayLabel->setScaledContents(true); // Ensure content scales with the window
    setCentralWidget(displayLabel);

    // Create QMediaPlayer and QVideoWidget for video playback
    player = new QMediaPlayer(this);
    videoWidget = new QVideoWidget(displayLabel); // Attach to QLabel
    player->setVideoOutput(videoWidget);

    // Initialize timers
    playTimer = new QTimer(this);
    pauseTimer = new QTimer(this);

    // Connect timer signals to appropriate slots
    connect(playTimer, &QTimer::timeout, this, &MainWindow::pauseAndShowImage);
    connect(pauseTimer, &QTimer::timeout, this, &MainWindow::resumeVideo);

    // Load media files
    loadMediaFiles();

    // Start playing videos
    playNextVideo();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadMediaFiles()
{
    // Get the user's home directory dynamically
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    // Define the relative directories for videos and images
    QString videoDirPath = homeDir + "/VideoImage/Videos";
    QString imageDirPath = homeDir + "/Downloads/images";

    // Retrieve the saved paths from application settings (if they exist)
    QSettings settings;
    videoDirPath = settings.value("media/videoDir", videoDirPath).toString();
    imageDirPath = settings.value("media/imageDir", imageDirPath).toString();

    // If paths are not saved, ask the user to choose them
    if (videoDirPath.isEmpty()) {
        videoDirPath = QFileDialog::getExistingDirectory(this, "Select Video Directory", QString(),
                                                         QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        settings.setValue("media/videoDir", videoDirPath);  // Save the selected path
    }

    if (imageDirPath.isEmpty()) {
        imageDirPath = QFileDialog::getExistingDirectory(this, "Select Image Directory", QString(),
                                                         QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        settings.setValue("media/imageDir", imageDirPath);  // Save the selected path
    }

    QDir videoDir(videoDirPath);
    QDir imageDir(imageDirPath);

    // Check if the directories exist
    if (!videoDir.exists() || !imageDir.exists()) {
        qDebug() << "One or both of the directories do not exist!";
        return;
    }

    // Get the list of all video files (.mpg, .mp4, .avi, etc.)
    QStringList videoFilters;
    videoFilters << "*.mpg" << "*.mp4" << "*.avi" << "*.mkv";  // Add any other video formats you need
    QFileInfoList videoFiles = videoDir.entryInfoList(videoFilters, QDir::Files);

    // Get the list of all image files (.png, .jpg, .jpeg, etc.)
    QStringList imageFilters;
    imageFilters << "*.png" << "*.jpg" << "*.jpeg";  // Add any other image formats you need
    QFileInfoList imageFiles = imageDir.entryInfoList(imageFilters, QDir::Files);

    // Clear previous lists (if needed)
    videoPaths.clear();
    imagePaths.clear();

    // Add video file paths to the videoPaths list
    for (const QFileInfo &fileInfo : videoFiles) {
        videoPaths << fileInfo.absoluteFilePath();
    }

    // Add image file paths to the imagePaths list
    for (const QFileInfo &fileInfo : imageFiles) {
        imagePaths << fileInfo.absoluteFilePath();
    }
}

void MainWindow::playNextVideo()
{
    if (currentVideoIndex >= videoPaths.size()) {
        // End of playlist, reset counters or stop playback as needed
        currentVideoIndex = 0;
        return;
    }

    QString fileName = videoPaths[currentVideoIndex];
    videoWidget->show();
    player->setSource(QUrl::fromLocalFile(fileName));
    player->play();

    playTimer->start(10000);  // Wait for 10 seconds before pausing the video
}

void MainWindow::pauseAndShowImage()
{
    player->pause();
    videoWidget->hide();

    // Display the first image
    if (currentImageIndex < imagePaths.size()) {
        QPixmap pixmap(imagePaths[currentImageIndex]);
        displayLabel->setPixmap(pixmap);
        displayLabel->setScaledContents(true);
        displayLabel->show();
    }

    // Increment to the next image
    currentImageIndex++;
    if (currentImageIndex >= imagePaths.size()) {
        currentImageIndex = 0;  // Loop back to the first image
    }

    // Wait for 2.5 seconds before showing the second image
    QTimer::singleShot(2500, this, [this]() {
        // Display the second image
        if (currentImageIndex < imagePaths.size()) {
            QPixmap pixmap(imagePaths[currentImageIndex]);
            displayLabel->setPixmap(pixmap);
            displayLabel->setScaledContents(true);
            displayLabel->show();
        }

        // Increment to the next image for the next cycle
        currentImageIndex++;
        if (currentImageIndex >= imagePaths.size()) {
            currentImageIndex = 0;  // Loop back to the first image
        }

        // Start the pauseTimer for the next cycle
        pauseTimer->start(2500);  // Show second image for the next 2.5 seconds
    });

    // Start the timer to show the first image for 2.5 seconds
    pauseTimer->start(2500);  // 2.5 seconds for the first image
}

void MainWindow::resumeVideo()
{
    displayLabel->clear();
    videoWidget->show();
    player->play();

    pauseTimer->stop();
    playTimer->start(10000);  // Wait for 10 seconds before pausing again

    currentVideoIndex++;
    if (currentVideoIndex >= videoPaths.size()) {
        currentVideoIndex = 0;
    }
}

void MainWindow::mediaFinished()
{
    playNextVideo();  // When video finishes, play the next one
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    videoWidget->setGeometry(displayLabel->geometry());
}
