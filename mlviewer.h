#ifndef MLVIEWER_H
#define MLVIEWER_H

#include <QMainWindow>
#include <QtWidgets>

#include "logwindow.h"


class mlviewer : public QMainWindow
{
    Q_OBJECT

private slots:
    void loadModel();
    void saveParam();
    void loadInput();
    void runThreshold();
    void rerunThreshold();
    void printThreshold();
    void showThresholdOutput(int exitCode);
    void showNewThresholdOutput(int exitCode);
    void runOutput();
    void printOutput();
    void showOutputOut(int exitCode);
    void resetAll();

public:
    mlviewer(QWidget *parent = 0);
    ~mlviewer();

    QSize main_size;
    QSize size_hint_top;

    QDockWidget *topPanel_model;
    QDockWidget *output_panel;
    QDockWidget *topPanel;
    QDockWidget *topPanel_buttons;
    QDockWidget *topPanel_log;
    QDockWidget *input_panel;
    QDockWidget *threshold_panel;
    QStatusBar *sBar;

    QLabel *load_model;
    QLabel *model_path_loaded;
    QLabel *bench;
    QFrame *line_resume;

    QGridLayout *grid_model;
    QString saved_model_path;
    QString saved_model;
    QString optimizer;
    QString learning_rate;

    QSpinBox *nb_images_w;
    QCheckBox *digits_per_img_w;
    QCheckBox *print_prob_w;
    QPushButton *confirm_param;
    QGridLayout *grid_param;

    QString model_version;
    QString nb_images;
    QString digits_per_img;
    QString print_prob;

    QPushButton *load_img;
    QPushButton *run_th;
    QPushButton *run_out;
    QPushButton *reset;

    QGridLayout *grid_input;
    QString folder_string_input;
    QLabel *folder_path_input;

    QGridLayout *grid_threshold;
    QString folder_string_threshold;
    QLabel *folder_path_threshold;
    QSlider *slider_threshold;

    QGridLayout *grid_output;
    QString folder_string_output;
    QLabel *folder_path_output;
    QString model_path;

    QStringList filenames_input;
    QStringList filenames_threshold;
    QStringList filenames_output;

    LogWindow *log;
    QProcess *p;
    QProgressBar *progress_preproc;
    int timer_preproc = 0;

    int bench_choice;

    int  chooseBenchmark();
    void showToolTip(QDockWidget *panel);
    void showTopPanel(QDockWidget *panel, QDockWidget *panel_model);
    void showTopButtons(QDockWidget *panel, QDockWidget *panel_param);
    void showTopLog(QDockWidget *panel, QDockWidget *panel_topbuttons);
    void showInput(QDockWidget *panel);
    void showThreshold(QDockWidget *panel);
    void showOutput(QDockWidget *panel);
    void showStatusBar();
};

#endif // MLVIEWER_H
