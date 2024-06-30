#include "kapprovalwidget.h"

KApprovalWidget::KApprovalWidget(QWidget *parent)
	: QWidget(parent)
	,m_database(KDataBase::getInstance())
	,m_worker(new KDatabaseWorker())
{
	ui = new Ui::KApprovalWidget();
	ui->setupUi(this);
	m_worker->moveToThread(&m_queryThread);
	// 注册自定义类型，方便跨线程传输
	qRegisterMetaType<QVector<KWaitApproval>>("QVector<KWaitApproval>");
	connect(&m_queryThread, &QThread::finished, m_worker, &QObject::deleteLater);
	connect(this, &KApprovalWidget::startAllReservationQuery, m_worker, &KDatabaseWorker::performRoomQuery);
	connect(m_worker, &KDatabaseWorker::roomsQueryFinished, this, &KApprovalWidget::handleAllReservationQueryResults);
	connect(this, &KApprovalWidget::startApprovalReservationQuery, m_worker, &KDatabaseWorker::performApprovalReservationQuery);
	connect(m_worker, &KDatabaseWorker::approvalReservationQueryFinished, this, &KApprovalWidget::handleApprovalReservationQueryResults);
	m_queryThread.start();
}

KApprovalWidget::~KApprovalWidget()
{
	delete ui;
	m_queryThread.quit();
	m_queryThread.wait();
}

// 审批预约
void KApprovalWidget::approvalReservation(int reservationID, bool isPassed)
{
	if (m_database.approvalReservation(reservationID, isPassed))
	{
		if (isPassed)
			QMessageBox::information(this, "提示", QString("预约已审批通过, 预约ID：%1").arg(reservationID));
		else
			QMessageBox::warning(this, "错误", QString("预约已驳回, 预约ID：%1").arg(reservationID));
	}
	else
	{
		QMessageBox::warning(this, "错误", QString("操作失败，请重试，预约ID：%1").arg(reservationID));
	}
	on_showApprovalReserve_clicked();
}

// 查看所有预约信息
void KApprovalWidget::on_showAllReservation_clicked()
{
	emit startAllReservationQuery();
}

// 查看所有待审批信息
void KApprovalWidget::on_showApprovalReserve_clicked()
{

	ui->listWidgetApprovalReserve->clear();
	emit startApprovalReservationQuery();
}

// 处理所有预约信息查询结果
void KApprovalWidget::handleAllReservationQueryResults(const QString results)
{
	if (results == "Error")
	{
		QMessageBox::warning(this, "错误", "查询失败！请重试");
	}
	else if (results.isEmpty())
	{
		QMessageBox::information(this, "提示", "暂无会议室信息");
	}
	else
	{
		ui->textBrowserAllReserve->clear();
		ui->textBrowserAllReserve->setText("所有预约信息如下：");
		ui->textBrowserAllReserve->append(results);
	}
}

// 处理待审批的预约信息查询结果
void KApprovalWidget::handleApprovalReservationQueryResults(const QVector<KWaitApproval> results)
{
	if (results.isEmpty())
	{
		QMessageBox::information(this, "提示", "待审批信息为空");
	}
	else if (results.first().reserveID == -1)
	{
		QMessageBox::warning(this, "错误", "查询失败，请重试");
	}
	else
	{
		for (const auto& reservation : results)
		{
			QString displayText = QString("预约ID: %1, 预约人用户名: %2, 预约人姓名: %3, 会议室: %4, 预约时间: %5 %6, 状态: %7")
				.arg(reservation.reserveID)
				.arg(reservation.userName)
				.arg(reservation.name)
				.arg(reservation.roomName)
				.arg(reservation.week)
				.arg(reservation.day)
				.arg(reservation.status);
			QListWidgetItem* item = new QListWidgetItem(displayText, ui->listWidgetApprovalReserve);
			item->setData(Qt::UserRole, reservation.reserveID); // 存储预约ID
		}
		connect(ui->agreeReserve, &QPushButton::clicked, [this]() {
			// 在此匿名函数中，现在可以使用 this->ui->listWidgetApprovalReserve 来访问listWidget
			if (ui->listWidgetApprovalReserve->selectedItems().isEmpty())
			{
				QMessageBox::warning(this, "提示", "请先选择一个预约以进行审批");
			}
			else
			{
				for (QListWidgetItem* item : ui->listWidgetApprovalReserve->selectedItems())
				{
					approvalReservation(item->data(Qt::UserRole).toInt(), true);
					break;
				}
			}
			});
		connect(ui->refuseReserve, &QPushButton::clicked, [this]() {
			// 在此匿名函数中，现在可以使用 this->ui->listWidgetApprovalReserve 来访问listWidget
			if (ui->listWidgetApprovalReserve->selectedItems().isEmpty())
			{
				QMessageBox::warning(this, "提示", "请先选择一个预约以进行审批");
			}
			else
			{
				for (QListWidgetItem* item : ui->listWidgetApprovalReserve->selectedItems())
				{
					approvalReservation(item->data(Qt::UserRole).toInt(), false);
					break;
				}
			}
			});
	}
}

void KApprovalWidget::on_logout_clicked()
{
	this->close();
	emit logout();
}
