#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QStatusBar>
#include <QPushButton>

class ObjectForgeMainWindow : public QMainWindow {
public:
    ObjectForgeMainWindow() {
        setWindowTitle("ObjectForge - 3D AI Reconstruction Studio");
        resize(1280, 720);

        // Main central layout
        QWidget *centralWidget = new QWidget(this);
        QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

        // Left Panel: Controls & Status
        QWidget *sidebar = new QWidget(this);
        sidebar->setFixedWidth(300);
        QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);

        QLabel *titleLabel = new QLabel("<b>ObjectForge Studio</b>", sidebar);
        QPushButton *connectGprcBtn = new QPushButton("Connect AI Backend", sidebar);
        QPushButton *startCameraBtn = new QPushButton("Start Camera Feed", sidebar);

        sidebarLayout->addWidget(titleLabel);
        sidebarLayout->addWidget(connectGprcBtn);
        sidebarLayout->addWidget(startCameraBtn);
        sidebarLayout->addStretch();

        // Right Panel: Camera / 3D Viewport Placeholder
        QWidget *viewport = new QWidget(this);
        viewport->setStyleSheet("background-color: #1e1e1e; border: 1px solid #333;");
        QVBoxLayout *viewportLayout = new QVBoxLayout(viewport);
        
        QLabel *viewportText = new QLabel("<h3 style='color: #888;'>Camera Viewport Placeholder</h3>", viewport);
        viewportText->setAlignment(Qt::AlignCenter);
        viewportLayout->addWidget(viewportText);

        // Assemble Layout
        mainLayout->addWidget(sidebar);
        mainLayout->addWidget(viewport);

        setCentralWidget(centralWidget);

        // Status Bar
        statusBar()->showMessage("ObjectForge Shell Initialized | Backend: Disconnected");
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ObjectForgeMainWindow window;
    window.show();
    return app.exec();
}