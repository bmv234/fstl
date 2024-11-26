#include "shaderlightprefs.h"
#include "canvas.h"
#include <QDoubleValidator>
#include <QApplication>

const QString ShaderLightPrefs::PREFS_GEOM = "prefsGeometry";

ShaderLightPrefs::ShaderLightPrefs(QWidget* parent, Canvas* _canvas)
    : QDialog(parent), canvas(_canvas)
{
    setWindowTitle("Shader Light Preferences");

    auto* prefsLayout = new QVBoxLayout(this);
    auto* middleWidget = new QWidget;

    auto* title = new QLabel("Shader preferences");
    QFont boldFont = QApplication::font();
    boldFont.setBold(true);
    title->setFont(boldFont);
    title->setAlignment(Qt::AlignCenter);

    prefsLayout->addWidget(title);

    auto* middleLayout = new QGridLayout(middleWidget);

    middleLayout->addWidget(new QLabel("Ambient Color"), 0, 0);
    middleLayout->addWidget(new QLabel("Directive Color"), 1, 0);
    middleLayout->addWidget(new QLabel("Direction"), 2, 0);

    QPixmap dummy(16, 16);
    dummy.fill(canvas->getAmbientColor());

    buttonAmbientColor = new QPushButton;
    buttonAmbientColor->setIcon(QIcon(dummy));
    buttonAmbientColor->setIconSize(QSize(32, 32));
    buttonAmbientColor->setFocusPolicy(Qt::NoFocus);
    connect(buttonAmbientColor, &QPushButton::clicked, this, &ShaderLightPrefs::buttonAmbientColorClicked);

    editAmbientFactor = new QLineEdit;
    editAmbientFactor->setValidator(new QDoubleValidator);
    editAmbientFactor->setText(QString("%1").arg(canvas->getAmbientFactor()));

    connect(editAmbientFactor, &QLineEdit::editingFinished, this, &ShaderLightPrefs::editAmbientFactorFinished);

    auto* buttonResetAmbientColor = new QPushButton("Reset");
    buttonResetAmbientColor->setMaximumWidth(60);
    buttonResetAmbientColor->setFocusPolicy(Qt::NoFocus);
    connect(buttonResetAmbientColor, &QPushButton::clicked, this, &ShaderLightPrefs::resetAmbientColorClicked);

    middleLayout->addWidget(buttonAmbientColor, 0, 1);
    middleLayout->addWidget(editAmbientFactor, 0, 2);
    middleLayout->addWidget(buttonResetAmbientColor, 0, 3);

    dummy.fill(canvas->getDirectiveColor());
    buttonDirectiveColor = new QPushButton;
    buttonDirectiveColor->setIcon(QIcon(dummy));
    buttonDirectiveColor->setIconSize(QSize(32, 32));
    buttonDirectiveColor->setFocusPolicy(Qt::NoFocus);
    connect(buttonDirectiveColor, &QPushButton::clicked, this, &ShaderLightPrefs::buttonDirectiveColorClicked);

    editDirectiveFactor = new QLineEdit;
    editDirectiveFactor->setValidator(new QDoubleValidator);
    editDirectiveFactor->setText(QString("%1").arg(canvas->getDirectiveFactor()));

    connect(editDirectiveFactor, &QLineEdit::editingFinished, this, &ShaderLightPrefs::editDirectiveFactorFinished);

    auto* buttonResetDirectiveColor = new QPushButton("Reset");
    buttonResetDirectiveColor->setMaximumWidth(60);
    buttonResetDirectiveColor->setFocusPolicy(Qt::NoFocus);
    connect(buttonResetDirectiveColor, &QPushButton::clicked, this, &ShaderLightPrefs::resetDirectiveColorClicked);

    middleLayout->addWidget(buttonDirectiveColor, 1, 1);
    middleLayout->addWidget(editDirectiveFactor, 1, 2);
    middleLayout->addWidget(buttonResetDirectiveColor, 1, 3);

    comboDirections = new QComboBox;
    comboDirections->setMaximumWidth(200);
    comboDirections->addItems(canvas->getNameDir());
    comboDirections->setCurrentIndex(canvas->getCurrentLightDirection());
    connect(comboDirections, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ShaderLightPrefs::comboDirectionsChanged);

    auto* buttonResetDirection = new QPushButton("Reset");
    buttonResetDirection->setMaximumWidth(60);
    buttonResetDirection->setFocusPolicy(Qt::NoFocus);
    connect(buttonResetDirection, &QPushButton::clicked, this, &ShaderLightPrefs::resetDirection);

    middleLayout->addWidget(comboDirections, 2, 1, 1, 2);
    middleLayout->addWidget(buttonResetDirection, 2, 3);

    prefsLayout->addWidget(middleWidget);

    auto* boxButton = new QWidget;
    auto* boxButtonLayout = new QHBoxLayout(boxButton);

    auto* spacerL = new QFrame;
    spacerL->setFrameShape(QFrame::HLine);

    auto* okButton = new QPushButton("Ok");
    okButton->setMaximumWidth(100);
    prefsLayout->addWidget(boxButton);
    okButton->setFocusPolicy(Qt::NoFocus);
    connect(okButton, &QPushButton::clicked, this, &ShaderLightPrefs::okButtonClicked);

    boxButtonLayout->addStretch();
    boxButtonLayout->addWidget(okButton);
    boxButtonLayout->addStretch();

    QSettings settings;
    if (!settings.value(PREFS_GEOM).isNull()) {
        restoreGeometry(settings.value(PREFS_GEOM).toByteArray());
    }
}

void ShaderLightPrefs::buttonAmbientColorClicked()
{
    QColor color = QColorDialog::getColor(canvas->getAmbientColor(), this);
    if (color.isValid()) {
        canvas->setAmbientColor(color);
        QPixmap dummy(16, 16);
        dummy.fill(color);
        buttonAmbientColor->setIcon(QIcon(dummy));
    }
}

void ShaderLightPrefs::editAmbientFactorFinished()
{
    canvas->setAmbientFactor(editAmbientFactor->text().toDouble());
}

void ShaderLightPrefs::resetAmbientColorClicked()
{
    canvas->resetAmbientColor();
    QPixmap dummy(16, 16);
    dummy.fill(canvas->getAmbientColor());
    buttonAmbientColor->setIcon(QIcon(dummy));
    editAmbientFactor->setText(QString("%1").arg(canvas->getAmbientFactor()));
}

void ShaderLightPrefs::buttonDirectiveColorClicked()
{
    QColor color = QColorDialog::getColor(canvas->getDirectiveColor(), this);
    if (color.isValid()) {
        canvas->setDirectiveColor(color);
        QPixmap dummy(16, 16);
        dummy.fill(color);
        buttonDirectiveColor->setIcon(QIcon(dummy));
    }
}

void ShaderLightPrefs::editDirectiveFactorFinished()
{
    canvas->setDirectiveFactor(editDirectiveFactor->text().toDouble());
}

void ShaderLightPrefs::resetDirectiveColorClicked()
{
    canvas->resetDirectiveColor();
    QPixmap dummy(16, 16);
    dummy.fill(canvas->getDirectiveColor());
    buttonDirectiveColor->setIcon(QIcon(dummy));
    editDirectiveFactor->setText(QString("%1").arg(canvas->getDirectiveFactor()));
}

void ShaderLightPrefs::comboDirectionsChanged(int ind)
{
    canvas->setCurrentLightDirection(ind);
}

void ShaderLightPrefs::resetDirection()
{
    canvas->resetCurrentLightDirection();
    comboDirections->setCurrentIndex(canvas->getCurrentLightDirection());
}

void ShaderLightPrefs::okButtonClicked()
{
    close();
}

void ShaderLightPrefs::resizeEvent(QResizeEvent* event)
{
    QSettings().setValue(PREFS_GEOM, saveGeometry());
    QDialog::resizeEvent(event);
}

void ShaderLightPrefs::moveEvent(QMoveEvent* event)
{
    QSettings().setValue(PREFS_GEOM, saveGeometry());
    QDialog::moveEvent(event);
}
