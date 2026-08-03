#include "ClickableLabel.h"
#include <QPainter>
#include <QToolTip>
#include <QApplication>

ClickableLabel::ClickableLabel(QWidget *parent) : QLabel(parent) {
    setMouseTracking(true); // REQUIRED: tracks movement without click
    setFocusPolicy(Qt::StrongFocus); // REQUIRED: label must accept key events
    setAlignment(Qt::AlignCenter);
    setStyleSheet("background-color: #1e1e1e; color: #888888; border-radius: 6px;");
    setText("Camera Viewport Placeholder");
}

void ClickableLabel::updateCandidates(const std::vector<cv::Rect>& boxes) {
    candidates_.clear();
    for (size_t i = 0; i < boxes.size(); ++i) {
        TargetCandidate tc;
        tc.id = static_cast<int>(i);
        tc.originalRect = boxes[i];
        tc.isHovered = (tc.id == hoveredId_);
        candidates_.push_back(tc);
    }
    updateScaledRects(); // Map them to current label size
    update(); // trigger paint
}

cv::Rect ClickableLabel::getHoveredRect() const {
    if (hoveredId_ >= 0 && hoveredId_ < candidates_.size()) {
        return candidates_[hoveredId_].originalRect;
    }
    return cv::Rect();
}

bool ClickableLabel::hasHoveredCandidate() const {
    return (hoveredId_ >= 0);
}

void ClickableLabel::mouseMoveEvent(QMouseEvent *event) {
    lastMousePos_ = event->pos();
    determineHoverStatus(event->pos());
    update(); // Redraw with highlight/tooltip
    QLabel::mouseMoveEvent(event);
}

void ClickableLabel::keyPressEvent(QKeyEvent *event) {
    if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) && hasHoveredCandidate()) {
        emit targetValidated(getHoveredRect());
    }
    QLabel::keyPressEvent(event);
}

void ClickableLabel::paintEvent(QPaintEvent *event) {
    QLabel::paintEvent(event); // draw image first

    if (pixmap().isNull() || candidates_.isEmpty()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Calculate letterbox offsets (code consolidated from previous step)
    QSize pixSize = pixmap().size();
    QSize lblSize = size();
    int offsetX = (lblSize.width() - pixSize.width()) / 2;
    int offsetY = (lblSize.height() - pixSize.height()) / 2;

    for (const auto& tc : candidates_) {
        QRect highlightRect = tc.scaledRect.translated(offsetX, offsetY);
        
        if (tc.isHovered) {
            // Draw bright Cyan Highlight for hovered box
            painter.setPen(QPen(QColor(0, 255, 255), 3)); 
            
            // Show Tooltip next to cursor
            QToolTip::showText(mapToGlobal(lastMousePos_), 
                               "Press ENTER to Lock Target", this, highlightRect);
        } else {
            // Draw standard green box for others
            painter.setPen(QPen(QColor(0, 255, 0), 1, Qt::DotLine));
        }
        painter.drawRect(highlightRect);
    }
}

void ClickableLabel::leaveEvent(QEvent *event) {
    hoveredId_ = -1;
    QToolTip::hideText();
    update();
    QLabel::leaveEvent(event);
}

void ClickableLabel::updateScaledRects() {
    if (pixmap().isNull() || candidates_.isEmpty()) return;

    QSize pixSize = pixmap().size();
    float scaleX = static_cast<float>(pixSize.width()) / 1280.0f;
    float scaleY = static_cast<float>(pixSize.height()) / 720.0f;

    for (auto& tc : candidates_) {
        tc.scaledRect = QRect(
            static_cast<int>(tc.originalRect.x * scaleX),
            static_cast<int>(tc.originalRect.y * scaleY),
            static_cast<int>(tc.originalRect.width * scaleX),
            static_cast<int>(tc.originalRect.height * scaleY)
        );
    }
}

void ClickableLabel::determineHoverStatus(const QPoint& mousePos) {
    int newHoveredId = -1;
    
    // consolidated logic: find letterbox click and check if inside scaled rect
    QSize pixSize = pixmap().size();
    int offsetX = (size().width() - pixSize.width()) / 2;
    int offsetY = (size().height() - pixSize.height()) / 2;

    for (int i = 0; i < candidates_.size(); ++i) {
        QRect translatedRect = candidates_[i].scaledRect.translated(offsetX, offsetY);
        if (translatedRect.contains(mousePos)) {
            newHoveredId = i;
            break; // found one
        }
    }

    // Update hover flags in candidates vector
    hoveredId_ = newHoveredId;
    for (int i = 0; i < candidates_.size(); ++i) {
        candidates_[i].isHovered = (i == hoveredId_);
    }
}