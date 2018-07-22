#include "mlviewer.h"

#define SIZE_PER_PICT 90

mlviewer::mlviewer(QWidget *parent)
    : QMainWindow(parent)
{
    this->setWindowTitle("MLViewer");
    this->showMaximized();

    main_size = QSize(QApplication::desktop()->availableGeometry().width(),
                      QApplication::desktop()->availableGeometry().height());

    topPanel_model     = new QDockWidget(NULL, this);
    output_panel       = new QDockWidget(NULL, this);
    topPanel           = new QDockWidget(NULL, this);
    topPanel_buttons   = new QDockWidget(NULL, this);
    topPanel_log       = new QDockWidget(NULL, this);
    input_panel        = new QDockWidget(NULL, this);
    threshold_panel    = new QDockWidget(NULL, this);

    bench_choice = chooseBenchmark();
    mlviewer::showToolTip(topPanel_model);
    mlviewer::showTopPanel(topPanel, topPanel_model);
    mlviewer::showTopButtons(topPanel_buttons, topPanel);
    mlviewer::showTopLog(topPanel_log, topPanel_buttons);
    mlviewer::showInput(input_panel);
    if (bench_choice == 0)
        mlviewer::showThreshold(threshold_panel);
    mlviewer::showOutput(output_panel);
    mlviewer::showStatusBar();
}

mlviewer::~mlviewer(){}

void mlviewer::loadModel()
{
    saved_model_path = QFileDialog::getOpenFileName(this,
            tr("Load your model"), QDir::currentPath(),
            tr("Saved Model (*.pb)"));
    if (saved_model_path != NULL)
    {
        QFileInfo fi(saved_model_path);
        QDir dir(fi.dir());

        QString model_version_tmp(dir.path());
        int vers = saved_model_path.lastIndexOf("/");
        model_version = model_version_tmp.mid(vers+1, model_version_tmp.length());

        int sm_x = saved_model_path.lastIndexOf("/");
        saved_model = saved_model_path.mid(sm_x+1, saved_model_path.length());

        model_path = QString(dir.path());
    }
}

void mlviewer::saveParam()
{
    nb_images = QString::number(nb_images_w->value());
    if (digits_per_img_w->isChecked())
        digits_per_img = QString::number(1);
    else
        digits_per_img = QString::number(0);
    if (print_prob_w->isChecked())
        print_prob = "True";
    else
        print_prob = "False";
    log->appendMessage("(II)     Parameters saved", 0);
    sBar->showMessage("Parameters saved", 5000);
    load_img->setEnabled(true);
}

void mlviewer::loadInput()
{
    filenames_input.clear();
    filenames_input = QFileDialog::getOpenFileNames(this,
                tr("Load your input"),QDir::currentPath(),
                tr("Images (*.png *.PNG *.jpg *.JPEG)"));
    if (!filenames_input.isEmpty())
    {
        int nb_inputs = filenames_input.count();
        if (nb_inputs != nb_images.toInt())
        {
            log->appendMessage("(EE)  Incoherence between number of images loaded and parameters"
                               " (" + QString::number(nb_inputs) + " vs. " + nb_images + ")", 0);
            sBar->showMessage("Incoherence between number of images loaded and parameters"
                            " (" + QString::number(nb_inputs) + " vs. " + nb_images + ")", 5000);
        }else
        {
            folder_path_input->deleteLater();
            if (nb_inputs == 1)
            {
                log->appendMessage("(II)     " + QString::number(nb_inputs) +" image loaded", 0);
                sBar->showMessage(QString::number(nb_inputs)+ " image loaded", 5000);
            }else
            {
                log->appendMessage("(II)     " + QString::number(nb_inputs) +" images loaded", 0);
                sBar->showMessage(QString::number(nb_inputs)+ " images loaded", 5000);
            }

            int max_size = 3;
            int evol_size = 3;
            int i = 0; int j = 0;
            for (int t = 0; t < nb_inputs; t++)
            {
                QImageReader reader(filenames_input.at(t));
                reader.setAutoTransform(true);
                const QImage newImage = reader.read();
                QLabel *myLabel = new QLabel("");
                if (nb_images.toInt() == 1)
                    if (newImage.width() > main_size.width()/3)
                        myLabel->setPixmap(QPixmap::fromImage(
                                newImage.scaled(main_size.width()/3,
                                                main_size.width()/3,
                                                Qt::KeepAspectRatio)));
                    else
                        myLabel->setPixmap(QPixmap::fromImage(
                                newImage));
                else
                    myLabel->setPixmap(QPixmap::fromImage(
                            newImage.scaled(SIZE_PER_PICT,
                                            SIZE_PER_PICT,
                                            Qt::KeepAspectRatio)));
                if (newImage.isNull())
                {
                    log->appendMessage("(EE)  Error opening image", 0);
                }else
                {
                    if (t > evol_size)
                    {
                        j += 1;
                        i  = 0;
                        evol_size += max_size+1;
                    }
                    grid_input->addWidget(myLabel, j, i, Qt::AlignCenter);
                    log->appendMessage("(II)     Image loaded: " + filenames_input.at(t), 0);
                    i += 1;
                }
            }
            run_th->setEnabled(true);
            reset->setEnabled(true);
            load_img->setEnabled(false);

            if (bench_choice == 1)
                run_out->setEnabled(true);
        }

    }else
    {
        log->appendMessage("(EE)  No images loaded", 0);
        sBar->showMessage("No images loaded", 5000);
    }
}

void mlviewer::runThreshold()
{
    progress_preproc = new QProgressBar();
    progress_preproc->setRange(0, 100);
    progress_preproc->setValue(0);

    progress_preproc->setMaximumHeight(sBar->sizeHint().height()-5);
    progress_preproc->setMaximumWidth(main_size.width()/6);
    progress_preproc->setTextVisible(false);
    sBar->addPermanentWidget(progress_preproc);

    p = new QProcess(this);
    QObject::connect(p, SIGNAL(readyReadStandardOutput()), this, SLOT(printThreshold()));
    QObject::connect(p, SIGNAL(readyReadStandardError()), this, SLOT(printThreshold()));
    QObject::connect(p, SIGNAL(finished(int)), this, SLOT(showThresholdOutput(int)));

    QStringList params;

    QString prog("./scripts/mnist_preproc.py");
    QString nb_img("--nb_images=" + nb_images);
    QString img_pth("--images_path=");
    for (int i = 0; i < filenames_input.count(); i++)
    {
        img_pth += filenames_input.at(i);
        if (i != filenames_input.count()-1)
            img_pth += ",";
    }

    params << prog.toStdString().c_str();
    params << nb_img.toStdString().c_str();
    params << img_pth.toStdString().c_str();

    log->appendMessage("(II)     Starting Threshold...\n", 0);
    p->start("python3", params);
}

void mlviewer::rerunThreshold()
{
    progress_preproc->setValue(0);
    progress_preproc->show();

    p = new QProcess(this);
    QObject::connect(p, SIGNAL(readyReadStandardOutput()), this, SLOT(printThreshold()));
    QObject::connect(p, SIGNAL(readyReadStandardError()), this, SLOT(printThreshold()));
    QObject::connect(p, SIGNAL(finished(int)), this, SLOT(showNewThresholdOutput(int)));

    QStringList params;

    QString prog("./scripts/mnist_preproc.py");
    QString nb_img("--nb_images=" + nb_images);
    QString th_value_string = QString::number(slider_threshold->value());
    QString val_th("--value_th=" + th_value_string);
    QString img_pth("--images_path=");
    for (int i = 0; i < filenames_input.count(); i++)
    {
        img_pth += filenames_input.at(i);
        if (i != filenames_input.count()-1)
            img_pth += ",";
    }

    params << prog.toStdString().c_str();
    params << nb_img.toStdString().c_str();
    params << val_th.toStdString().c_str();
    params << img_pth.toStdString().c_str();

    log->appendMessage("(II)     Starting Threshold...\n", 0);
    p->start("python3", params);
}

void mlviewer::printThreshold()
{
    QString printed_tmp = p->readAllStandardOutput();
    QString perc_tmp    = p->readAllStandardError();
    int perc            = perc_tmp.toInt();
    int perc_cur        = progress_preproc->value()+perc;

    if (timer_preproc == 0)
        sBar->showMessage("Opening pre-processing...", 3000);
    else
        sBar->showMessage("Running threshold...", 1000);
    timer_preproc++;

    if (printed_tmp != "")
    {
        log->appendMessage(printed_tmp.toStdString().c_str(), 1);
    }

    if (perc_tmp != "")
    {
        progress_preproc->setValue(perc_cur);
//        progress_preproc->setMinimum(0); // Infinite Progress
//        progress_preproc->setMaximum(0); // Infinite Progress
    }

}

void mlviewer::showThresholdOutput(int exitCode)
{
    if (exitCode == 0)
    {
        confirm_param->setEnabled(false);
        load_img->setEnabled(false);
        run_th->setEnabled(false);

        timer_preproc = 0;

        progress_preproc->setValue(100);
        log->appendMessage("(II)     Threshold done", 0);
        sBar->showMessage("Threshold done", 5000);

        filenames_threshold.clear();
        for (int t = 0; t < nb_images.toInt(); t++)
        {
            filenames_threshold << ("./build-project/image_" + QString::number(t) + ".png");
        }
        folder_path_threshold->deleteLater();
        int max_size = 3;
        int evol_size = 3;
        int i = 0; int j = 0;
        for (int t = 0; t < nb_images.toInt(); t++)
        {
            QImageReader reader(filenames_threshold.at(t));
            reader.setAutoTransform(true);
            const QImage newImage = reader.read();
            QLabel *myLabel = new QLabel("");
            if (nb_images.toInt() == 1)
                if (newImage.width() > main_size.width()/3)
                    myLabel->setPixmap(QPixmap::fromImage(
                            newImage.scaled(main_size.width()/3,
                                            main_size.width()/3,
                                            Qt::KeepAspectRatio)));
                else
                    myLabel->setPixmap(QPixmap::fromImage(
                            newImage));
            else
                myLabel->setPixmap(QPixmap::fromImage(
                        newImage.scaled(SIZE_PER_PICT,
                                        SIZE_PER_PICT,
                                        Qt::KeepAspectRatio)));
            if (newImage.isNull())
            {
                log->appendMessage("(EE)  Error opening image", 0);
            }else
            {
                if (t > evol_size)
                {
                    j += 1;
                    i  = 0;
                    evol_size += max_size+1;
                }
                grid_threshold->addWidget(myLabel, j, i, Qt::AlignCenter);
                log->appendMessage("(II)     Image loaded: " + filenames_threshold.at(t), 0);
                i += 1;
            }
        }

        slider_threshold = new QSlider(Qt::Horizontal);
        slider_threshold->setMinimumWidth(main_size.width()/5);
        slider_threshold->setRange(0, 255);
        slider_threshold->setValue(127);
        if (nb_images.toInt() == 1)
            grid_threshold->addWidget(slider_threshold, j+1, 0, j+1, 1, Qt::AlignCenter);
        else
            grid_threshold->addWidget(slider_threshold, j+1, 0, j+1, 4, Qt::AlignCenter);

        QObject::connect(slider_threshold, SIGNAL(sliderReleased()), this, SLOT(rerunThreshold()));

        run_out->setEnabled(true);
        progress_preproc->hide();
    }
}

void mlviewer::showNewThresholdOutput(int exitCode)
{
    if (exitCode == 0)
    {
        timer_preproc = 0;

        progress_preproc->setValue(100);
        log->appendMessage("(II)     Threshold done", 0);
        sBar->showMessage("Threshold done", 5000);

        filenames_threshold.clear();
        for (int t = 0; t < nb_images.toInt(); t++)
        {
            filenames_threshold << ("./build-project/image_" + QString::number(t) + ".png");
        }

        int max_size = 3;
        int evol_size = 3;
        int i = 0; int j = 0;
        for (int t = 0; t < nb_images.toInt(); t++)
        {
            QImageReader reader(filenames_threshold.at(t));
            reader.setAutoTransform(true);
            const QImage newImage = reader.read();
            QLabel *myLabel = new QLabel("");
            if (nb_images.toInt() == 1)
                if (newImage.width() > main_size.width()/3)
                    myLabel->setPixmap(QPixmap::fromImage(
                            newImage.scaled(main_size.width()/3,
                                            main_size.width()/3,
                                            Qt::KeepAspectRatio)));
                else
                    myLabel->setPixmap(QPixmap::fromImage(
                            newImage));
            else
                myLabel->setPixmap(QPixmap::fromImage(
                        newImage.scaled(SIZE_PER_PICT,
                                        SIZE_PER_PICT,
                                        Qt::KeepAspectRatio)));
            if (newImage.isNull())
            {
                log->appendMessage("(EE)  Error opening image", 0);
            }else
            {
                if (t > evol_size)
                {
                    j += 1;
                    i  = 0;
                    evol_size += max_size+1;
                }
                grid_threshold->addWidget(myLabel, j, i, Qt::AlignCenter);
                log->appendMessage("(II)     Image loaded: " + filenames_threshold.at(t), 0);
                i += 1;
            }
        }
    progress_preproc->hide();
    }
}

void mlviewer::runOutput()
{
    if (bench_choice == 1)
    {
        progress_preproc = new QProgressBar();
        progress_preproc->setRange(0, 100);
        progress_preproc->setMaximumHeight(sBar->sizeHint().height()-5);
        progress_preproc->setMaximumWidth(main_size.width()/6);
        progress_preproc->setTextVisible(false);
        sBar->addPermanentWidget(progress_preproc);
    }
    progress_preproc->setValue(0);
    progress_preproc->show();

    p = new QProcess(this);
    QObject::connect(p, SIGNAL(readyReadStandardOutput()), this, SLOT(printOutput()));
    QObject::connect(p, SIGNAL(readyReadStandardError()), this, SLOT(printOutput()));
    QObject::connect(p, SIGNAL(finished(int)), this, SLOT(showOutputOut(int)));

    QStringList params;

    QString prog;
    QString mod_pth;
    if (bench_choice == 0)
    {
        prog = "./scripts/mnist_load.py";
        mod_pth = "--model_path="+model_path+"/";
    }else
    {
        prog = "./scripts/vgg16.py";
        mod_pth = "--model_path="+saved_model_path;
    }
    QString nb_img("--nb_images=" + nb_images);
    QString dig_im("--digits_per_img=" + digits_per_img);

    QString img_pth("--images_path=");

    params << prog.toStdString().c_str();
    params << nb_img.toStdString().c_str();
    if (bench_choice == 0)
    {
        params << dig_im.toStdString().c_str();
    }else
    {
        for (int i = 0; i < filenames_input.count(); i++)
        {
            img_pth += filenames_input.at(i);
            if (i != filenames_input.count()-1)
                img_pth += ",";
        }
        params << img_pth.toStdString().c_str();
    }
    params << mod_pth.toStdString().c_str();

    log->appendMessage("(II)     Starting Output...\n", 0);
    p->start("python3", params);
}

void mlviewer::printOutput()
{
    QString printed_tmp = p->readAllStandardOutput();
    QString perc_tmp    = p->readAllStandardError();
    int perc            = perc_tmp.toInt();
    int perc_cur        = progress_preproc->value()+perc;

    if (timer_preproc == 0)
        sBar->showMessage("Opening computing...", 3000);
    else
        sBar->showMessage("Running output...", 1000);
    timer_preproc++;

    if (printed_tmp != "")
    {
        log->appendMessage(printed_tmp.toStdString().c_str(), 1);
    }

    if (perc_tmp != "")
    {
        progress_preproc->setValue(perc_cur);
//        progress_preproc->setMinimum(0); // Infinite Progress
//        progress_preproc->setMaximum(0); // Infinite Progress
    }

}

void mlviewer::showOutputOut(int exitCode)
{
    if (exitCode == 0)
    {
        confirm_param->setEnabled(false);
        load_img->setEnabled(false);
        run_th->setEnabled(false);
        run_out->setEnabled(false);

        timer_preproc = 0;

        progress_preproc->setValue(100);
        log->appendMessage("(II)     Output done", 0);
        sBar->showMessage("Output done", 5000);

        filenames_output.clear();
        for (int t = 0; t < nb_images.toInt(); t++)
        {
            filenames_output << ("./build-project/image_finish_" + QString::number(t) + ".png");
        }
        folder_path_output->deleteLater();
        int max_size = 3;
        int evol_size = 3;
        int i = 0; int j = 0;
        for (int t = 0; t < nb_images.toInt(); t++)
        {
            QImageReader reader(filenames_output.at(t));
            reader.setAutoTransform(true);
            const QImage newImage = reader.read();
            QLabel *myLabel = new QLabel("");
            if (nb_images.toInt() == 1)
                if (newImage.width() > main_size.width()/3)
                    myLabel->setPixmap(QPixmap::fromImage(
                            newImage.scaled(main_size.width()/3,
                                            main_size.width()/3,
                                            Qt::KeepAspectRatio)));
                else
                    myLabel->setPixmap(QPixmap::fromImage(
                            newImage));
            else
                myLabel->setPixmap(QPixmap::fromImage(
                        newImage.scaled(SIZE_PER_PICT,
                                        SIZE_PER_PICT,
                                        Qt::KeepAspectRatio)));
            if (newImage.isNull())
            {
                log->appendMessage("(EE)  Error opening image", 0);
            }else
            {
                if (t > evol_size)
                {
                    j += 1;
                    i  = 0;
                    evol_size += max_size+1;
                }
                grid_output->addWidget(myLabel, j, i, Qt::AlignCenter);
                log->appendMessage("(II)     Image loaded: " + filenames_output.at(t), 0);
                i += 1;
            }
        }

        run_out->setEnabled(false);
        progress_preproc->hide();
    }
}


void mlviewer::resetAll()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Reset");
    msgBox.setText("Are you sure you want to reset?");
    QPushButton *resetInput         = msgBox.addButton("Confirm", QMessageBox::ActionRole);
    QPushButton *abortButton        = msgBox.addButton(QMessageBox::Cancel);

    msgBox.exec();

    if (msgBox.clickedButton() == resetInput)
    {
        progress_preproc->hide();
        filenames_input.clear();
        while (grid_input->count())
        {
            QWidget* widget = grid_input->itemAt(0)->widget();
            if (widget)
            {
                grid_input->removeWidget(widget);
                delete widget;
            }
        }
        folder_string_input       = "No input image";
        folder_path_input         = new QLabel(folder_string_input);
        grid_input->addWidget(folder_path_input, 0, 0, 0, 0, Qt::AlignCenter);

        if (bench_choice == 0)
        {
            filenames_threshold.clear();
            while (grid_threshold->count())
            {
                QWidget* widget = grid_threshold->itemAt(0)->widget();
                if (widget)
                {
                    grid_threshold->removeWidget(widget);
                    delete widget;
                }
            }
            folder_string_threshold       = "No input image";
            folder_path_threshold         = new QLabel(folder_string_threshold);
            grid_threshold->addWidget(folder_path_threshold, 0, 0, 0, 0, Qt::AlignCenter);
        }
        filenames_output.clear();
        while (grid_output->count())
        {
            QWidget* widget = grid_output->itemAt(0)->widget();
            if (widget)
            {
                grid_output->removeWidget(widget);
                delete widget;
            }
        }
        folder_string_output       = "No input image";
        folder_path_output         = new QLabel(folder_string_output);
        grid_output->addWidget(folder_path_output, 0, 0, 0, 0, Qt::AlignCenter);
        log->appendMessage("(II)     Input images reset", 0);
        sBar->showMessage("Input images reset", 5000);

        confirm_param->setEnabled(true);
        load_img->setEnabled(false);
        run_th->setEnabled(false);
        run_out->setEnabled(false);
        reset->setEnabled(false);

        timer_preproc = 0;
    }
}

int mlviewer::chooseBenchmark()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("  Choose a neural network  ");
    QImageReader reader("./icons/chip_256.png");
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    msgBox.setIconPixmap(QPixmap::fromImage(newImage.scaled(main_size.width()/15,
                                                            main_size.width()/15,
                                                            Qt::KeepAspectRatio)));
    msgBox.setText("Welcome!\n\nPick the neural network you want to work with."
                   "\nYou will have to provide a neural network trained model.\n\n"
                   "\nHelp:\n\n"
                   "VGG:     \tClassification of images\n"
                   "MNIST: \tHandwritten Digits Recognition\n");
    QPushButton *mnist              = msgBox.addButton("MNIST", QMessageBox::ActionRole);
    QPushButton *vgg                = msgBox.addButton("VGG", QMessageBox::ActionRole);

    QObject::connect(mnist, SIGNAL(clicked(bool)), this, SLOT(loadModel()));
    QObject::connect(vgg, SIGNAL(clicked(bool)), this, SLOT(loadModel()));

    int ret = msgBox.exec();

    if (msgBox.clickedButton() == mnist)
    {
        bench = new QLabel("MNIST");
        return 0;
    }else if (msgBox.clickedButton() == vgg)
    {
        bench = new QLabel("VGG");
        return 1;
    }
}

void mlviewer::showToolTip(QDockWidget *panel)
{
    panel->setAllowedAreas(Qt::LeftDockWidgetArea);
    panel->setFloating(false);
    panel->setFeatures(QDockWidget::NoDockWidgetFeatures);

    QGroupBox *groupBox         = new QGroupBox("Model");

    QLabel *model_path          = new QLabel("<b>Model name:<\b>");
    load_model                  = new QLabel(saved_model);
    QLabel *benchmark_chose     = new QLabel("<b>Neural Network:<\b>");

    line_resume                 = new QFrame();
    line_resume->setFrameShape(QFrame::HLine);
    line_resume->setFrameShadow(QFrame::Sunken);
    QLabel *myLabel = new QLabel("");

    if (bench_choice ==0)
    {
        QImageReader reader("./icons/number-blocks.png");
        reader.setAutoTransform(true);
        const QImage newImage = reader.read();
        myLabel->setPixmap(QPixmap::fromImage(
                newImage.scaled(main_size.width()/24,
                                main_size.width()/24,
                                Qt::KeepAspectRatio)));
    }else
    {
        QImageReader reader("./icons/image.png");
        reader.setAutoTransform(true);
        const QImage newImage = reader.read();
        myLabel->setPixmap(QPixmap::fromImage(
                newImage.scaled(main_size.width()/26,
                                main_size.width()/26,
                                Qt::KeepAspectRatio)));
    }

    grid_model = new QGridLayout;
    grid_model->addWidget(benchmark_chose, 0, 0);
    grid_model->addWidget(bench, 0, 1);
    grid_model->addWidget(model_path, 1, 0);
    grid_model->addWidget(load_model, 1, 1);
    grid_model->addWidget(line_resume, 2, 0, 1, 2);
    grid_model->addWidget(myLabel, 3, 0, 1, 2, Qt::AlignCenter);
    grid_model->setVerticalSpacing(8);
    grid_model->setSizeConstraint(QLayout::SetFixedSize);
    groupBox->setLayout(grid_model);


    panel->setWidget(groupBox);
    addDockWidget(Qt::LeftDockWidgetArea, panel);
}

void mlviewer::showTopPanel(QDockWidget *panel, QDockWidget *panel_model)
{
    panel->setAllowedAreas(Qt::TopDockWidgetArea);
    panel->setFloating(false);
    panel->setFeatures(QDockWidget::NoDockWidgetFeatures);

    QGroupBox *groupBox         = new QGroupBox("Parameters");

    QLabel *nb_images_str       = new QLabel("<b>Number of images to deal with:<\b>");
    nb_images_w                 = new QSpinBox();
    nb_images_w->setRange(1, 50);
    QLabel *digits_per_img_str  = new QLabel("<b>Many digits per image:<\b>");
    digits_per_img_w            = new QCheckBox();
    QLabel *print_prob_str      = new QLabel("<b>Print probabilities:<\b>");
    print_prob_w                = new QCheckBox();
    print_prob_w->setChecked(true);
    QFrame* line_resume         = new QFrame();
    line_resume->setFrameShape(QFrame::HLine);
    line_resume->setFrameShadow(QFrame::Sunken);
    confirm_param               = new QPushButton("Save Parameters");
    confirm_param->setEnabled(true);

    grid_param = new QGridLayout;
    grid_param->addWidget(nb_images_str, 0, 0);
    grid_param->addWidget(nb_images_w, 0, 1);
    if (bench_choice ==0)
    {
        grid_param->addWidget(digits_per_img_str, 1, 0);
        grid_param->addWidget(digits_per_img_w, 1, 1);
        grid_param->addWidget(print_prob_str, 2, 0);
        grid_param->addWidget(print_prob_w, 2, 1);
        grid_param->addWidget(line_resume, 3, 0, 1, 2);
        grid_param->addWidget(confirm_param, 4, 0, 4, 2, Qt::AlignCenter);
        grid_param->setVerticalSpacing(6);
    }else
    {
        grid_param->addWidget(print_prob_str, 1, 0);
        grid_param->addWidget(print_prob_w, 1, 1);
        grid_param->addWidget(line_resume, 2, 0, 1, 2);
        grid_param->addWidget(confirm_param, 3, 0, 3, 2, Qt::AlignCenter);
    }
    grid_param->setSizeConstraint(QLayout::SetFixedSize);
    groupBox->setLayout(grid_param);

    panel->setWidget(groupBox);
    splitDockWidget(panel_model, panel, Qt::Horizontal);

    QObject::connect(confirm_param, SIGNAL(clicked(bool)), this, SLOT(saveParam()));
}

void mlviewer::showTopButtons(QDockWidget *panel, QDockWidget *panel_param)
{
    panel->setAllowedAreas(Qt::TopDockWidgetArea);
    panel->setFloating(false);
    panel->setFeatures(QDockWidget::NoDockWidgetFeatures);

    QGroupBox *groupBox         = new QGroupBox("Steps");

    load_img                    = new QPushButton("Load Input");
    load_img->setEnabled(false);
    run_th                      = new QPushButton("Run Threshold");
    run_th->setEnabled(false);
    run_out                     = new QPushButton("Run Inference");
    run_out->setEnabled(false);
    reset                       = new QPushButton("Reset");
    reset->setEnabled(false);

    QFrame* line_resume         = new QFrame();
    line_resume->setFrameShape(QFrame::HLine);
    line_resume->setFrameShadow(QFrame::Sunken);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(load_img, 0, 0);
    if (bench_choice == 0)
    {
        grid->addWidget(run_th, 1, 0);
        grid->addWidget(run_out, 2, 0);
        grid->addWidget(line_resume, 3, 0, 1, 2);
        grid->addWidget(reset, 4, 0, 1, 2, Qt::AlignCenter);
        grid->setVerticalSpacing(10);
    }else
    {
        grid->addWidget(run_out, 1, 0);
        grid->addWidget(line_resume, 2, 0, 1, 2);
        grid->addWidget(reset, 3, 0, 1, 2, Qt::AlignCenter);
        grid->setVerticalSpacing(10);
    }
    grid->setSizeConstraint(QLayout::SetFixedSize);
    groupBox->setLayout(grid);

    panel->setWidget(groupBox);
    splitDockWidget(panel_param, panel, Qt::Horizontal);

    QObject::connect(load_img, SIGNAL(clicked(bool)), this, SLOT(loadInput()));
    QObject::connect(run_th, SIGNAL(clicked(bool)), this, SLOT(runThreshold()));
    QObject::connect(run_out, SIGNAL(clicked(bool)), this, SLOT(runOutput()));
    QObject::connect(reset, SIGNAL(clicked(bool)), this, SLOT(resetAll()));
}

void mlviewer::showTopLog(QDockWidget *panel, QDockWidget *panel_topbuttons)
{
    panel->setAllowedAreas(Qt::TopDockWidgetArea);
    panel->setFloating(false);
    panel->setFeatures(QDockWidget::NoDockWidgetFeatures);

    QGroupBox *groupBox = new QGroupBox("Log");

    log = new LogWindow();

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(log, 0, 0);
    groupBox->setLayout(grid);

    panel->setWidget(groupBox);
    splitDockWidget(panel_topbuttons, panel, Qt::Horizontal);

    if (saved_model_path == NULL)
        log->appendMessage("(EE)  No model loaded", 0);
    else
        log->appendMessage("(II)     Found model: " + saved_model_path, 0);
}

void mlviewer::showInput(QDockWidget *panel)
{
    panel->setAllowedAreas(Qt::TopDockWidgetArea);
    panel->setFloating(false);
    panel->setFeatures(QDockWidget::NoDockWidgetFeatures);
    if (bench_choice == 0)
    {
        panel->setMinimumWidth((int)(main_size.width()/3));
        panel->setMaximumWidth((int)(main_size.width()/3));
    }else
    {
        panel->setMinimumWidth((int)(main_size.width()/2));
        panel->setMaximumWidth((int)(main_size.width()/2));
    }

    QGroupBox *groupBox         = new QGroupBox("Input");

    folder_string_input       = "No input image";
    folder_path_input         = new QLabel(folder_string_input);

    grid_input = new QGridLayout;
    grid_input->addWidget(folder_path_input, 0, 0, 0, 0, Qt::AlignCenter);
    groupBox->setLayout(grid_input);

    panel->setWidget(groupBox);
    addDockWidget(Qt::BottomDockWidgetArea, panel);
}

void mlviewer::showThreshold(QDockWidget *panel)
{
    panel->setAllowedAreas(Qt::TopDockWidgetArea);
    panel->setFloating(false);
    panel->setFeatures(QDockWidget::NoDockWidgetFeatures);

    QGroupBox *groupBox         = new QGroupBox("Threshold");

    folder_string_threshold       = "No input image";
    folder_path_threshold         = new QLabel(folder_string_threshold);
    QFrame* line_resume         = new QFrame();
    line_resume->setFrameShape(QFrame::HLine);
    line_resume->setFrameShadow(QFrame::Sunken);

    grid_threshold = new QGridLayout;
    grid_threshold->addWidget(folder_path_threshold, 0, 0, 0, 0, Qt::AlignCenter);
    groupBox->setLayout(grid_threshold);

    panel->setWidget(groupBox);
    addDockWidget(Qt::BottomDockWidgetArea, panel);
}

void mlviewer::showOutput(QDockWidget *panel)
{
    panel->setAllowedAreas(Qt::TopDockWidgetArea);
    panel->setFloating(false);
    panel->setFeatures(QDockWidget::NoDockWidgetFeatures);
    if (bench_choice == 0)
    {
        panel->setMinimumWidth((int)(main_size.width()/3));
        panel->setMaximumWidth((int)(main_size.width()/3));
    }else
    {
        panel->setMinimumWidth((int)(main_size.width()/2));
        panel->setMaximumWidth((int)(main_size.width()/2));
    }

    QGroupBox *groupBox         = new QGroupBox("Output");

    folder_string_output       = "No input image";
    folder_path_output         = new QLabel(folder_string_output);
    QFrame* line_resume         = new QFrame();
    line_resume->setFrameShape(QFrame::HLine);
    line_resume->setFrameShadow(QFrame::Sunken);

    grid_output = new QGridLayout;
    grid_output->addWidget(folder_path_output, 0, 0, 0, 0, Qt::AlignCenter);
    groupBox->setLayout(grid_output);

    panel->setWidget(groupBox);
    addDockWidget(Qt::BottomDockWidgetArea, panel);
}

void mlviewer::showStatusBar()
{
    sBar = statusBar();
    sBar->show();
    if (saved_model_path == NULL)
    {
        sBar->showMessage("No model loaded", 5000);
        bench->setText("None");
    }else
        sBar->showMessage("Found model", 5000);
}
