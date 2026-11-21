#include "cineforge_window.hpp"
#include "cineforge.hpp"

#include <QComboBox>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QStandardPaths>
#include <QThread>
#include <QMetaObject>
#include <QUrl>
#include <QVBoxLayout>

namespace {
QSlider* sliderRow(QFormLayout* form, const QString& title, int initial) {
    auto* slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 100);
    slider->setValue(initial);
    form->addRow(title, slider);
    return slider;
}
QString keyName(int index) {
    static const QStringList keys{"C","C♯","D","D♯","E","F","F♯","G","G♯","A","A♯","B"};
    return keys.value(index);
}
}

CineforgeWindow::CineforgeWindow(QWidget* parent) : QDialog(parent) {
    setWindowTitle("CINEFORGE — Hollywood Stem & Sample Engine");
    resize(980, 760);
    auto* root = new QVBoxLayout(this);
    auto* heading = new QLabel("<h1>CINEFORGE / HOLLYWOOD STEM & SAMPLE ENGINE</h1><p>Scene-to-Pack · cohesive sound families · offline 24-bit WAV rendering</p>");
    heading->setWordWrap(true);
    root->addWidget(heading);
    auto* scene = new QLineEdit("Dark industrial thriller, monumental metal, deep sub impacts");
    scene->setMaxLength(512);
    scene->setPlaceholderText("Describe the scene, energy and sonic character…");
    root->addWidget(scene);

    auto* grid = new QGridLayout();
    auto* bpm = new QSpinBox(); bpm->setRange(40, 200); bpm->setValue(92); bpm->setSuffix(" BPM");
    auto* key = new QComboBox(); key->addItems({"C","C♯","D","D♯","E","F","F♯","G","G♯","A","A♯","B"}); key->setCurrentIndex(5);
    auto* rate = new QComboBox(); rate->addItems({"48000","96000"});
    auto* count = new QSpinBox(); count->setRange(8, 12); count->setValue(8);
    auto* variations = new QSpinBox(); variations->setRange(1, 12); variations->setValue(3);
    auto* seed = new QSpinBox(); seed->setRange(0, 2147483647); seed->setValue(24011);
    grid->addWidget(new QLabel("TEMPO"),0,0); grid->addWidget(bpm,0,1);
    grid->addWidget(new QLabel("KEY"),0,2); grid->addWidget(key,0,3);
    grid->addWidget(new QLabel("SAMPLE RATE"),1,0); grid->addWidget(rate,1,1);
    grid->addWidget(new QLabel("PACK SIZE"),1,2); grid->addWidget(count,1,3);
    grid->addWidget(new QLabel("MICRO-TAKES"),2,0); grid->addWidget(variations,2,1);
    grid->addWidget(new QLabel("SEED"),2,2); grid->addWidget(seed,2,3);
    root->addLayout(grid);

    auto* controls = new QFormLayout();
    auto* mood = sliderRow(controls, "Mood / tonal character", 72);
    auto* space = sliderRow(controls, "Space / depth", 58);
    auto* punch = sliderRow(controls, "Punch / transient weight", 78);
    auto* controlBox = new QGroupBox("SCENE CONTROLS");
    controlBox->setLayout(controls);
    root->addWidget(controlBox);

    auto* pathRow = new QHBoxLayout();
    auto* output = new QLineEdit(QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/CINEFORGE");
    auto* browse = new QPushButton("Output folder…");
    pathRow->addWidget(output,1); pathRow->addWidget(browse);
    root->addLayout(pathRow);
    auto* actions = new QHBoxLayout();
    auto* generate = new QPushButton("GENERATE COHESIVE PACK");
    generate->setMinimumHeight(44);
    auto* preview = new QPushButton("Open combined preview");
    preview->setEnabled(false);
    actions->addWidget(generate,1); actions->addWidget(preview);
    root->addLayout(actions);
    auto* status = new QLabel("Ready · local deterministic synthesis · no network requests");
    root->addWidget(status);
    auto* progress = new QProgressBar();
    progress->setRange(0, 1); progress->setValue(0); progress->setVisible(false);
    root->addWidget(progress);
    auto* files = new QListWidget();
    root->addWidget(files,1);

    connect(browse,&QPushButton::clicked,this,[this,output]{
        const auto dir=QFileDialog::getExistingDirectory(this,"CINEFORGE output",output->text());
        if(!dir.isEmpty()) output->setText(dir);
    });
    connect(preview,&QPushButton::clicked,this,[preview]{
        const auto path=preview->property("path").toString();
        if(!path.isEmpty()) QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    });
    connect(files,&QListWidget::itemDoubleClicked,this,[](QListWidgetItem* item){
        const auto path=item->data(Qt::UserRole).toString();
        if(!path.isEmpty()) QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    });
    connect(generate,&QPushButton::clicked,this,[=]{
        ventu::cineforge::Settings s;
        s.scene=scene->text().toUtf8().toStdString(); s.bpm=bpm->value(); s.rootNote=key->currentIndex();
        s.sampleRate=rate->currentText().toInt(); s.packSize=count->value(); s.variations=variations->value();
        s.seed=seed->value(); s.mood=mood->value(); s.space=space->value(); s.punch=punch->value();
        const auto issue=ventu::cineforge::validate(s);
        if(!issue.empty()){QMessageBox::warning(this,"Invalid settings",QString::fromStdString(issue));return;}
        const auto dir=output->text().trimmed();
        if(dir.isEmpty()){QMessageBox::warning(this,"Output required","Choose an output folder.");return;}
        generate->setEnabled(false); preview->setEnabled(false); files->clear();
        generationPending_ = true;
        progress->setVisible(true); status->setText("Rendering coherent dry/wet families and 24-bit preview…");
        worker_=QThread::create([this,s,dir,generate,preview,files,status,progress]{
            const auto result=ventu::cineforge::generate(s,std::filesystem::u8path(dir.toUtf8().constData()));
            QMetaObject::invokeMethod(this,[=]{
                progress->setVisible(false);
                generationPending_ = false;
                generate->setEnabled(true);
                if(!result){status->setText("Generation failed: "+QString::fromStdString(result.error));return;}
                for(const auto& asset:result.assets){
                    auto* dry=new QListWidgetItem(QString::fromStdString(asset.name)+"  ·  Dry",files);
                    dry->setData(Qt::UserRole,QString::fromStdString(asset.dryPath.string()));
                    auto* wet=new QListWidgetItem(QString::fromStdString(asset.name)+"  ·  Wet",files);
                    wet->setData(Qt::UserRole,QString::fromStdString(asset.wetPath.string()));
                }
                auto* manifest=new QListWidgetItem("cineforge_manifest.json",files);
                manifest->setData(Qt::UserRole,QString::fromStdString(result.manifestPath.string()));
                preview->setEnabled(true);
                preview->setProperty("path",QString::fromStdString(result.previewPath.string()));
                status->setText(QString("Complete · %1 cohesive assets · 24-bit / %2 kHz · %3 BPM · %4")
                    .arg(result.assets.size()).arg(s.sampleRate/1000).arg(s.bpm).arg(keyName(s.rootNote)));
            },Qt::QueuedConnection);
        });
        connect(worker_,&QThread::finished,this,[this]{
            if(worker_){worker_->deleteLater();worker_=nullptr;}
        });
        worker_->start();
    });
}

void CineforgeWindow::closeEvent(QCloseEvent* event) {
    if (generationPending_) {
        event->ignore();
        return;
    }
    QDialog::closeEvent(event);
}
