#ifndef CELL_H
#define CELL_H

#include <QWidget>

#include "op.h"

class DynamicFontSizeLabel;
class QLabel;

class Cell : public QWidget
{
    Q_OBJECT
    friend class Grid;

public:
    Cell(const size_t x, const size_t y, QWidget* parent = nullptr) noexcept;
    virtual ~Cell() noexcept = default;

    void   set(const size_t& val) noexcept;
    size_t get(void) const noexcept { return _val; }

public slots:
    void setColorEffect(const QColor& c = QColor(255, 20, 20)) noexcept;
    void removeColorEffect() noexcept;

signals:
    void changed(Op);
    void hovered(bool);

protected:
    void enterEvent(QEvent*) override;
    void leaveEvent(QEvent*) override;
    void keyReleaseEvent(QKeyEvent*) override;

    void updateVal(size_t) noexcept;

private:
    static constexpr size_t _max_val{ 9 };

    const size_t _x, _y;
    size_t       _val{ 0 };

    // TODO - Add a Widget to show when hovered

    // UI related members
    QLabel*               _bg{ nullptr };
    DynamicFontSizeLabel* _lb{ nullptr };
};

#endif // CELL_H
