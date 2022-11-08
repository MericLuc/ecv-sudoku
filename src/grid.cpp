/**
 * @file grid.cpp
 * @brief Implementation of \a grid.h
 * @author lhm
 */

// Project's headers
#include "grid.h"
#include "cell.h"

// External headers
#include <ecv.hpp>

/*****************************************************************************/
Grid::Grid(QWidget* parent) noexcept
  : QWidget(parent)
{
    setFixedSize(500, 500);

    for (size_t i{ 0 }, k{ 0 }; i < _size; ++i)
        for (size_t j{ 0 }; j < _size; ++j, ++k)
            _cells[k] = new Cell(i, j, this);

    for (auto& c : _cells) {
        c->show();
        connect(c, SIGNAL(changed(Op)), this, SLOT(onCellChanged(Op)));
        connect(c, &Cell::hovered, []() { /* TODO */ });
    }
}

/*****************************************************************************/
bool
Grid::undo(void) noexcept
{
    if (std::empty(_done))
        return false;

    _undone.emplace_back(_done.back());
    _done.pop_back();

    if (std::size(_undone) > max_hist_size)
        _undone.pop_front();

    const auto op{ _undone.back().reverse() };
    op.apply();

    emit changed();

    return true;
}

/*****************************************************************************/
bool
Grid::redo(void) noexcept
{
    if (std::empty(_undone))
        return false;

    _done.emplace_back(_undone.back());
    _undone.pop_back();

    if (std::size(_done) > max_hist_size)
        _done.pop_front();

    const auto op{ _done.back() };
    op.apply();

    emit changed();

    return true;
}

/*****************************************************************************/
std::vector<std::string>
Grid::data() const noexcept
{
    auto ret{ std::vector<std::string>(_size, std::string(_size, ' ')) };

    for (size_t i{ 0 }, k{ 0 }; i < _size; ++i)
        for (size_t j{ 0 }; j < _size; ++j, ++k)
            ret[i][j] = _cells[k]->get() + '0';

    return ret;
}

/*****************************************************************************/
bool
Grid::fromData(const std::vector<std::string>& data) noexcept
{
    if (std::empty(data) || _size != std::size(data))
        return false;

    for (const auto& line : data)
        if (_size != std::size(line))
            return false;

    _done.clear();
    _undone.clear();

    for (size_t i{ 0 }, k{ 0 }; i < _size; ++i)
        for (size_t j{ 0 }; j < _size; ++j, ++k)
            _cells[k]->set(data[i][j] - '0');

    return true;
}

/*****************************************************************************/
void
Grid::clear(void) noexcept
{
    for (auto c : _cells)
        c->set(0);

    _done.clear();
    _undone.clear();

    emit changed();
}

/*****************************************************************************/
void
Grid::onCellChanged(Op op)
{
    _done.push_back(op);
    _undone.clear();

    if (std::size(_done) > max_hist_size)
        _done.pop_front();

    emit changed();
}
