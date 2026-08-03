#pragma once

#include <QLabel>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPoint>
#include <QVector>
#include <opencv2/opencv.hpp>

// Helper structure for tracking detections in the viewport
struct TargetCandidate {
    int id; // unique index for tracking
    cv::Rect originalRect; // rect in 1280x720 space
    QRect scaledRect;      // rect mapped to Qt label space
    bool isHovered;
};

class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(QWidget *parent = nullptr);
    ~ClickableLabel() override = default;

    // Updates the internal list of boxes so hover logic knows where they are
    void updateCandidates(const std::vector<cv::Rect>& boxes);
    
    // Returns the currently hovered original CV Rect, or empty if none
    cv::Rect getHoveredRect() const;
    
    // Check if any candidate is currently being hovered
    bool hasHoveredCandidate() const;

signals:
    // Emitted when Enter/Return is pressed while hovering a target
    void targetValidated(const cv::Rect& finalRect);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override; // custom drawing for tooltips
    void leaveEvent(QEvent *event) override;      // clear hover when mouse leaves viewport

private:
    void updateScaledRects(); // recalculate scaled rects on resize
    void determineHoverStatus(const QPoint& mousePos);

    QVector<TargetCandidate> candidates_;
    int hoveredId_{-1};
    QPoint lastMousePos_;
};