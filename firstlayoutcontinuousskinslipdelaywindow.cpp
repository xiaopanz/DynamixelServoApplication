#include "firstlayoutcontinuousskinslipdelaywindow.h"
#include "ui_firstlayoutcontinuousskinslipdelaywindow.h"

FirstLayoutContinuousSkinSlipDelayWindow::FirstLayoutContinuousSkinSlipDelayWindow(ServoUtility *servoUtilityPtr, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FirstLayoutContinuousSkinSlipDelayWindow),
    servoUtilityPtr(servoUtilityPtr),
    parent(parent)
{
    ui->setupUi(this);
    ui->startButton->setEnabled(false);
}

FirstLayoutContinuousSkinSlipDelayWindow::~FirstLayoutContinuousSkinSlipDelayWindow()
{
    delete ui;
}

void FirstLayoutContinuousSkinSlipDelayWindow::on_angleSlider_valueChanged(int value)
{
    ui->angleVal->setText(QString::number(value));
}


void FirstLayoutContinuousSkinSlipDelayWindow::on_velocitySlider_valueChanged(int value)
{
    ui->velocityVal->setText(QString::number(value));
}


void FirstLayoutContinuousSkinSlipDelayWindow::on_delaySlider_valueChanged(int value)
{
    ui->delayVal->setText(QString::number(value));
}


void FirstLayoutContinuousSkinSlipDelayWindow::on_resetButton_clicked()
{
    ui->angleSlider->setEnabled(false);
    ui->velocitySlider->setEnabled(false);
    ui->quitButton->setEnabled(false);
    ui->resetButton->setEnabled(false);

    for (int i = 0; i < NUM_OF_DXL_1; i++) {
        servoUtilityPtr->disableTorque(servoUtilityPtr->dxl_ids_1[i]);
        servoUtilityPtr->setOperatingMode(servoUtilityPtr->dxl_ids_1[i], 4);
        servoUtilityPtr->enableTorque(servoUtilityPtr->dxl_ids_1[i]);
        servoUtilityPtr->setVelocity(servoUtilityPtr->dxl_ids_1[i], DXL_VELOCITY_VALUE);
    }
    qDebug() << "Resetting servos positions...";
    servoUtilityPtr->resetPosition(servoUtilityPtr->dxl_ids_1, NUM_OF_DXL_1, ui->angleSlider->value() - 4096);
    qDebug() << "Servos positions have been reset!";
    for (int i = 0; i < NUM_OF_DXL_1; i++) {
        servoUtilityPtr->disableTorque(servoUtilityPtr->dxl_ids_1[i]);
        servoUtilityPtr->setOperatingMode(servoUtilityPtr->dxl_ids_1[i], 3);
        servoUtilityPtr->enableTorque(servoUtilityPtr->dxl_ids_1[i]);
        servoUtilityPtr->setVelocity(servoUtilityPtr->dxl_ids_1[i], ui->velocitySlider->value());
    }

    ui->startButton->setEnabled(true);
}


void FirstLayoutContinuousSkinSlipDelayWindow::on_startButton_clicked()
{
    ui->startButton->setEnabled(false);
    int direction = 0;
    int32_t dxls_present_position[NUM_OF_DXL_1];
    int dxls_goal_position_0[NUM_OF_DXL_1];
    int dxls_goal_position_1[NUM_OF_DXL_1];
    for (int i = 0; i < NUM_OF_DXL_1; i++) {
        dxls_present_position[i] = 0;
        dxls_goal_position_0[i] = 2048 - (ui->angleSlider->value() - 2048);
        dxls_goal_position_1[i] = ui->angleSlider->value();
    }
    int dxl_goal_position_0 = 2048 - (ui->angleSlider->value() - 2048);
    int dxl_goal_position_1 = ui->angleSlider->value();
    int temp_dxl_ids[NUM_OF_DXL_1];
    int j = 0;
    for (int i = NUM_OF_DXL_1 - 1; i >= 0; i--) {
        temp_dxl_ids[j] = servoUtilityPtr->dxl_ids_1[i];
        j++;
    }
    int delay = ui->delaySlider->value();

    int iteration = 0;
    while (iteration != 10) {
        if (direction == 0) {
            std::thread thread_sync_read(&ServoUtility::syncReadPosition, servoUtilityPtr, NUM_OF_DXL_1, servoUtilityPtr->dxl_ids_1, dxls_present_position, dxls_goal_position_0);
            std::thread thread_sync_write(&ServoUtility::syncWritePositionWithDelay, servoUtilityPtr, NUM_OF_DXL_1, temp_dxl_ids, dxl_goal_position_0, delay);
            thread_sync_write.join();
            thread_sync_read.join();
        }
        else {
            std::thread thread_sync_read(&ServoUtility::syncReadPosition, servoUtilityPtr, NUM_OF_DXL_1, servoUtilityPtr->dxl_ids_1, dxls_present_position, dxls_goal_position_1);
            std::thread thread_sync_write(&ServoUtility::syncWritePositionWithDelay, servoUtilityPtr, NUM_OF_DXL_1, servoUtilityPtr->dxl_ids_1, dxl_goal_position_1, delay);
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
    std::thread thread_sync_read(&ServoUtility::syncReadPosition, servoUtilityPtr, NUM_OF_DXL_1, servoUtilityPtr->dxl_ids_1, dxls_present_position, dxls_goal_position_0);
    std::thread thread_sync_write(&ServoUtility::syncWritePositionWithDelay, servoUtilityPtr, NUM_OF_DXL_1, temp_dxl_ids, dxl_goal_position_0, delay);
    thread_sync_write.join();
    thread_sync_read.join();
    // Change servos velocity
    for (int i = 0; i < NUM_OF_DXL_1; i++) {
        servoUtilityPtr->setVelocity(servoUtilityPtr->dxl_ids_1[i], DXL_VELOCITY_VALUE);
    }
    ui->angleSlider->setEnabled(true);
    ui->velocitySlider->setEnabled(true);
    ui->delaySlider->setEnabled(true);
    ui->resetButton->setEnabled(true);
    ui->quitButton->setEnabled(true);
    qDebug() << "Finished performing continuous skin slip with delay!";
}


void FirstLayoutContinuousSkinSlipDelayWindow::on_quitButton_clicked()
{
    ui->startButton->setEnabled(false);
    // Change servos velocity
    for (int i = 0; i < NUM_OF_DXL_1; i++) {
        servoUtilityPtr->setVelocity(servoUtilityPtr->dxl_ids_1[i], DXL_VELOCITY_VALUE);
    }
    qDebug() << "Resetting servos positions...";
    servoUtilityPtr->resetPosition(servoUtilityPtr->dxl_ids_1, NUM_OF_DXL_1, 0);
    qDebug() << "Servos positions have been reset!";
    ui->angleSlider->setValue(2600);
    ui->velocitySlider->setValue(100);
    ui->delaySlider->setValue(200);
    this->hide();
    parent->show();
}

