#include "thirdlayoutcontinuoussqueezeandpushwindow.h"
#include "ui_thirdlayoutcontinuoussqueezeandpushwindow.h"

ThirdLayoutContinuousSqueezeAndPushWindow::ThirdLayoutContinuousSqueezeAndPushWindow(ServoUtility *servoUtilityPtr, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ThirdLayoutContinuousSqueezeAndPushWindow),
    servoUtilityPtr(servoUtilityPtr),
    parent(parent)
{
    ui->setupUi(this);
    ui->startButton->setEnabled(false);
}

ThirdLayoutContinuousSqueezeAndPushWindow::~ThirdLayoutContinuousSqueezeAndPushWindow()
{
    delete ui;
}

void ThirdLayoutContinuousSqueezeAndPushWindow::on_angleSlider_valueChanged(int value)
{
    ui->angleVal->setText(QString::number(value));
}


void ThirdLayoutContinuousSqueezeAndPushWindow::on_velocitySlider_valueChanged(int value)
{
    ui->velocityVal->setText(QString::number(value));
}


void ThirdLayoutContinuousSqueezeAndPushWindow::on_resetButton_clicked()
{
    ui->angleSlider->setEnabled(false);
    ui->velocitySlider->setEnabled(false);
    ui->quitButton->setEnabled(false);
    ui->resetButton->setEnabled(false);

    for (int i = 0; i < NUM_OF_DXL_3; i++) {
        servoUtilityPtr->disableTorque(servoUtilityPtr->dxl_ids_3[i]);
        servoUtilityPtr->setOperatingMode(servoUtilityPtr->dxl_ids_3[i], 4);
        servoUtilityPtr->enableTorque(servoUtilityPtr->dxl_ids_3[i]);
        servoUtilityPtr->setVelocity(servoUtilityPtr->dxl_ids_3[i], DXL_VELOCITY_VALUE);
    }

    qDebug() << "Resetting servos positions...";
    servoUtilityPtr->resetPositionCustomThirdLayout(servoUtilityPtr->dxl_ids_3, 2048, firstReset, 0);
    firstReset = 0;
    qDebug() << "Servos positions have been reset!";

    for (int i = 0; i < NUM_OF_DXL_3; i++) {
        servoUtilityPtr->disableTorque(servoUtilityPtr->dxl_ids_3[i]);
        servoUtilityPtr->setOperatingMode(servoUtilityPtr->dxl_ids_3[i], 3);
        servoUtilityPtr->enableTorque(servoUtilityPtr->dxl_ids_3[i]);
        servoUtilityPtr->setVelocity(servoUtilityPtr->dxl_ids_3[i], ui->velocitySlider->value());
    }
    ui->startButton->setEnabled(true);
}


void ThirdLayoutContinuousSqueezeAndPushWindow::on_startButton_clicked()
{
    ui->startButton->setEnabled(false);
    int direction = 0;
    int32_t dxls_present_position[NUM_OF_DXL_3];
    int dxls_goal_position_0[NUM_OF_DXL_3];
    int dxls_goal_position_1[NUM_OF_DXL_3];
    for (int i = 0; i < NUM_OF_DXL_3; i++) {
        dxls_present_position[i] = 0;
        if (i < NUM_OF_DXL_3 / 2) {
            dxls_goal_position_0[i] =2048;
            dxls_goal_position_1[i] =2048 - (ui->angleSlider->value() - 2048);
        }
        else {
            dxls_goal_position_0[i] = 2048;
            dxls_goal_position_1[i] = ui->angleSlider->value();
        }
    }

    int iteration = 0;
    while (iteration != 10) {
        if (direction == 0) {
            std::thread thread_sync_read(&ServoUtility::syncReadPosition, servoUtilityPtr, NUM_OF_DXL_3, servoUtilityPtr->dxl_ids_3, dxls_present_position, dxls_goal_position_0);
            std::thread thread_sync_write(&ServoUtility::syncWritePosition, servoUtilityPtr, NUM_OF_DXL_3, servoUtilityPtr->dxl_ids_3, dxls_goal_position_0);
            thread_sync_write.join();
            thread_sync_read.join();
        }
        else {
            std::thread thread_sync_read(&ServoUtility::syncReadPosition, servoUtilityPtr, NUM_OF_DXL_3, servoUtilityPtr->dxl_ids_3, dxls_present_position, dxls_goal_position_1);
            std::thread thread_sync_write(&ServoUtility::syncWritePosition, servoUtilityPtr, NUM_OF_DXL_3, servoUtilityPtr->dxl_ids_3, dxls_goal_position_1);
            thread_sync_write.join();
            thread_sync_read.join();
        }
        if (direction == 0) {
            direction = 1;
        }
        else {
            direction = 0;
        }
        iteration++;
    }

    // Change servos velocity
    for (int i = 0; i < NUM_OF_DXL_3; i++) {
        servoUtilityPtr->setVelocity(servoUtilityPtr->dxl_ids_3[i], DXL_VELOCITY_VALUE);
    }
    ui->angleSlider->setEnabled(true);
    ui->velocitySlider->setEnabled(true);
    ui->resetButton->setEnabled(true);
    ui->quitButton->setEnabled(true);
    qDebug() << "Finished performing continuous squeeze and push!";
}


void ThirdLayoutContinuousSqueezeAndPushWindow::on_quitButton_clicked()
{
    ui->startButton->setEnabled(false);
    if (firstReset == 1) {
        // Change servos velocity
        for (int i = 0; i < NUM_OF_DXL_3; i++) {
            servoUtilityPtr->setVelocity(servoUtilityPtr->dxl_ids_3[i], DXL_VELOCITY_VALUE);
        }
        qDebug() << "Resetting servos positions...";
        servoUtilityPtr->resetPositionThirdLayout(servoUtilityPtr->dxl_ids_3, 0);
        qDebug() << "Servos positions have been reset!";
    } else {
        for (int i = 0; i < NUM_OF_DXL_3; i++) {
            servoUtilityPtr->disableTorque(servoUtilityPtr->dxl_ids_3[i]);
            servoUtilityPtr->setOperatingMode(servoUtilityPtr->dxl_ids_3[i], 4);
            servoUtilityPtr->enableTorque(servoUtilityPtr->dxl_ids_3[i]);
            servoUtilityPtr->setVelocity(servoUtilityPtr->dxl_ids_3[i], DXL_VELOCITY_VALUE);
        }
        qDebug() << "Resetting servos positions...";
        servoUtilityPtr->quitCustomThirdLayout(servoUtilityPtr->dxl_ids_3, 0);
        firstReset = 1;
        qDebug() << "Servos positions have been reset!";
        for (int i = 0; i < NUM_OF_DXL_3; i++) {
            servoUtilityPtr->disableTorque(servoUtilityPtr->dxl_ids_3[i]);
            servoUtilityPtr->setOperatingMode(servoUtilityPtr->dxl_ids_3[i], 3);
            servoUtilityPtr->enableTorque(servoUtilityPtr->dxl_ids_3[i]);
            servoUtilityPtr->setVelocity(servoUtilityPtr->dxl_ids_3[i], DXL_VELOCITY_VALUE);
        }
    }
    ui->angleSlider->setValue(2600);
    ui->velocitySlider->setValue(100);
    this->hide();
    parent->show();
}

