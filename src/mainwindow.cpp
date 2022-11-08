/**
 * @file mainwindow.cpp
 * @brief Implementation of \a mainwindow.h
 * @author lhm
 */

// Project's headers
#include "mainwindow.h"
#include "./ui_mainwindow.h"

// External headers
#include <ecv.hpp>

// Standard headers
#include <chrono>
#include <random>

// Qt headers
#include <QGraphicsDropShadowEffect>
#include <QIntValidator>

static auto                         _sols{ ecv::Sudoku::generate()->solve(1) };
static std::unique_ptr<ecv::Sudoku> _model{ nullptr };

/*****************************************************************************/
void
setupWidget(QWidget* w, const QString& name = "setupWidget")
{
    QGraphicsDropShadowEffect* shadow_effect = new QGraphicsDropShadowEffect;
    shadow_effect->setOffset(1, 1);
    shadow_effect->setColor(Qt::gray);
    shadow_effect->setBlurRadius(2);

    w->setObjectName(name);
    w->setGraphicsEffect(shadow_effect);
}

/*****************************************************************************/
/*!
 * @brief Very basic stopwatch to measure ellapsed time since it has been created
 */
template<class Clock = std::chrono::high_resolution_clock, class Unit = std::chrono::milliseconds>
class stopwatch
{
public:
    explicit stopwatch() noexcept
      : _start{ Clock::now() }
    {}

    Unit elapsed() const noexcept
    {
        return std::chrono::duration_cast<Unit>(Clock::now() - _start);
    }

private:
    typedef std::chrono::time_point<Clock> TimePt;
    const TimePt                           _start;
};

/*****************************************************************************/
class CLineEdit : public QWidget
{
public:
    explicit CLineEdit(QLineEdit* le) noexcept
      : QWidget(le)
      , _le{ le }
      , _txt{ _le->text() }
    {
        _le->installEventFilter(this);
    }

protected:
    bool eventFilter(QObject* obj, QEvent* e) override
    {
        if (obj == _le) {
            if (QEvent::FocusIn == e->type()) {
                _txt = _le->text();
                _le->clear();
            } else if (QEvent::FocusOut == e->type() && _le->text().isEmpty()) {
                _le->setText(_txt);
            }
        }
        return false;
    }

private:
    QLineEdit* _le;
    QString    _txt;
};

/*****************************************************************************/
MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->res_gb->setEnabled(false);

    ui->le_sols->setValidator(new QIntValidator(1, 1000000, this));
    new CLineEdit(ui->le_sols);

    ui->le_nbcells->setValidator(new QIntValidator(1, 81, this));
    new CLineEdit(ui->le_nbcells);

    for (auto& button :
         { ui->new_pb, ui->solve_pb, ui->undo_cb, ui->redo_cb, ui->res_pb, ui->pb_random })
        setupWidget(button);

    setupWidget(ui->le_sols);
    setupWidget(ui->le_nbcells);

    setStyleSheet("QLineEdit#setupWidget{background-color:#ffffff;color:#5F5F5F;}"
                  "QLineEdit#setupWidget:hover{background-color:#daffb3;}"
                  "QPushButton#setupWidget{background-color:#ffffff;color:#5F5F5F;}"
                  "QPushButton#setupWidget:hover{background-color:#daffb3;}"
                  "QPushButton#setupWidget:pressed{background-color:#5cb300;}"
                  "QPushButton#setupWidget:!enabled{background-color:rgb(200, 200, 200);}"
                  "QComboBox#setupWidget{background-color:#ffffff;color:#5F5F5F;}"
                  "QComboBox#setupWidget:hover{background-color:#daffb3;}"
                  "QComboBox#setupWidget:pressed{background-color:#5cb300;}"
                  "QComboBox#setupWidget:!enabled{background-color:rgb(200, 200, 200);}");

    for (auto& button : { ui->undo_cb, ui->redo_cb })
        button->setStyleSheet(
          QString("border-top-right-radius:%1px;border-bottom-right-radius:%1px;").arg(3));

    // Connect to grid changes
    connect(ui->square_w, &Grid::changed, this, [this]() {
        if (ui->res_gb->isEnabled()) {
            ui->res_gb->setDisabled(true);
            ui->res_label->clear();
        }
    });

    // Connect buttons with Grid
    connect(ui->undo_cb, SIGNAL(clicked()), ui->square_w, SLOT(undo()));
    connect(ui->redo_cb, SIGNAL(clicked()), ui->square_w, SLOT(redo()));
    connect(ui->new_pb, &QPushButton::clicked, this, [this]() { ui->square_w->clear(); });
    connect(ui->res_pb, &QPushButton::clicked, this, [this]() {
        if (nullptr == _model || std::empty(_sols))
            return;

        static std::random_device dev;
        static std::mt19937       rng{ dev() };

        std::uniform_int_distribution<std::mt19937::result_type> dist{ 1, std::size(_sols) - 1 };

        ui->centralwidget->setDisabled(true);
        ui->centralwidget->repaint();

        ui->square_w->fromData(_model->apply(_sols[dist(rng)]));

        ui->centralwidget->setDisabled(false);
        ui->centralwidget->repaint();
    });
    connect(ui->solve_pb, &QPushButton::clicked, this, [this]() {
        ui->centralwidget->setGraphicsEffect(new QGraphicsBlurEffect);
        ui->centralwidget->setDisabled(true);
        ui->centralwidget->repaint();

        // Perform the ecv resolution
        _sols.clear();
        _model.reset(nullptr);
        _model = ecv::Sudoku::generate(ui->square_w->data());
        if (nullptr == _model)
            return;

        stopwatch watch;
        _sols = _model->solve(ui->le_sols->text().toULong());

        if (auto solsNb{ std::size(_sols) }; solsNb > 0) {
            ui->res_pb->setEnabled(solsNb > 1);
            ui->res_gb->setEnabled(true);
            ui->res_label->setText(QString("Found %1 solutions (%2 ms)")
                                     .arg(std::size(_sols))
                                     .arg(watch.elapsed().count()));
            ui->square_w->fromData(_model->apply(_sols[0]));
        } else {
            ui->res_label->setText("No solution");
        }

        ui->centralwidget->setGraphicsEffect(nullptr);
        ui->centralwidget->setDisabled(false);
        ui->centralwidget->repaint();
    });
    connect(ui->pb_random, &QPushButton::clicked, this, [this]() {
        ui->centralwidget->setGraphicsEffect(new QGraphicsBlurEffect);
        ui->centralwidget->setDisabled(true);
        ui->centralwidget->repaint();

        _model.reset(nullptr);
        _sols.clear();

        _model = ecv::Sudoku::generate();
        _sols = _model->solve(10000);

        static std::random_device dev;
        static std::mt19937       rng{ dev() };

        std::uniform_int_distribution<std::mt19937::result_type> dist{ 1, std::size(_sols) - 1 };
        static std::uniform_int_distribution<std::mt19937::result_type> idxes{ 0, 8 };

        ui->centralwidget->setDisabled(true);
        ui->centralwidget->repaint();

        auto st{ _model->apply(_sols[dist(rng)]) };
        auto cmpt{ 0 };
        while (cmpt < (81 - std::min(81, ui->le_nbcells->text().toInt()))) {
            if (auto& c{ st[idxes(rng)][idxes(rng)] }; '0' != c) {
                c = '0';
                ++cmpt;
            }
        }
        ui->square_w->fromData(st);

        ui->centralwidget->setGraphicsEffect(nullptr);
        ui->centralwidget->setDisabled(false);
        ui->centralwidget->repaint();
    });
}

/*****************************************************************************/
MainWindow::~MainWindow()
{
    delete ui;
}
