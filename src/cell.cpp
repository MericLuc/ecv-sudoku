/**
 * @file cell.cpp
 * @brief Implementation of \a cell.h
 * @author lhm
 */

#include "cell.h"

// Project's headers
#include "DynamicFontSizeLabel.h"
#include "op.h"

// Qt headers
#include <QGraphicsDropShadowEffect>
#include <QKeyEvent>
#include <QLabel>

// External headers

/*****************************************************************************/
Cell::Cell(const size_t x, const size_t y, QWidget* parent) noexcept
  : QWidget(parent)
  , _x{ x }
  , _y{ y }
  , _bg{ new QLabel(this) }
  , _lb{ new DynamicFontSizeLabel(this) }
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setUpdatesEnabled(false);

    auto pSize{ parent->size() };
    auto cSize{ QSize(pSize.width() / _max_val, pSize.height() / _max_val) };

    // Position
    move(_y * cSize.width(), _x * cSize.height());
    setFixedSize(cSize);

    // Setup label to contain value
    int lbMargin{ cSize.width() / 5 - 5 };
    int lbSize{ cSize.width() - 2 * lbMargin };
    _lb->setAlignment(Qt::AlignCenter);
    _lb->setObjectName("cellLabel");
    _lb->setFixedSize(lbSize, lbSize);
    _lb->move(lbMargin, lbMargin);
    setStyleSheet(
      QString("#cellLabel{border-radius:%1px;background-color:transparent;}").arg(lbSize / 2));

    // Setup background
    QString style("background-color:#f1f2f3;border:1px solid #000000;");
    if ((0 == _x || _max_val - 1 == _x) && (0 == _y || _max_val - 1 == _y)) {
        style += QString("border-%1-%2-radius:%3px;")
                   .arg(_x == 0 ? "top" : "bottom", _y == 0 ? "left" : "right")
                   .arg(cSize.width() / 5);
    }
    if ((2 == _x || 5 == _x)) {
        style += QString("border-bottom:3px solid;");
    }
    if ((2 == _y || 5 == _y)) {
        style += QString("border-right:3px solid #000000;");
    }
    _bg->setFixedSize(cSize);
    _bg->setStyleSheet(style);

    setUpdatesEnabled(true);
}

/*****************************************************************************/
void
Cell::enterEvent(QEvent*)
{
    setFocus(Qt::FocusReason::OtherFocusReason);

    auto lb_effect{ new QGraphicsDropShadowEffect() };

    // Setup hovering effect on labeld
    lb_effect->setOffset(.0);
    lb_effect->setBlurRadius(20.0);
    lb_effect->setColor(Qt::red);

    _lb->setGraphicsEffect(lb_effect);

    emit hovered(true);
}

/*****************************************************************************/
void
Cell::leaveEvent(QEvent*)
{
    _lb->setGraphicsEffect(nullptr);

    emit hovered(false);
}

/*****************************************************************************/
void
Cell::keyReleaseEvent(QKeyEvent* e)
{
    auto val{ e->text() };
    auto ok{ false };

    if (auto intVal{ val.toUInt(&ok) }; ok) {
        if (intVal <= _max_val)
            updateVal(intVal);
    }
    QWidget::keyReleaseEvent(e);
}

/*****************************************************************************/
void
Cell::updateVal(size_t v) noexcept
{
    if (_val == v)
        return;

    Op op{ _val, v, this };

    _val = v;
    _lb->setText((0 == _val) ? "" : QString::number(_val));

    update();
    emit changed(op);
}

/*****************************************************************************/
void
Cell::set(const size_t& val) noexcept
{
    if (_val == val)
        return;

    _val = val;
    _lb->setText((0 == _val) ? "" : QString::number(_val));

    update();
}
