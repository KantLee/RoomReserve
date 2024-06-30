#include "kcoderwidget.h"

KCoderWidget::KCoderWidget(QString userName, QWidget *parent)
	: QWidget(parent)
	,m_database(KDataBase::getInstance())
	,m_userName(userName)
	,m_worker(new KDatabaseWorker())
{
	ui = new Ui::KCoderWidget();
	ui->setupUi(this);
	ui->rooms->clear();
	ui->rooms->setText("请先查询会议室信息！");
	m_worker->moveToThread(&m_queryThread);
	// 注册自定义类型，方便跨线程传输
	qRegisterMetaType<QVector<KPersonalReserve>>("QVector<KPersonalReserve>");
	connect(&m_queryThread, &QThread::finished, m_worker, &QObject::deleteLater);
	connect(this, &KCoderWidget::startSearchRooms, m_worker, &KDatabaseWorker::performSearchRooms);
	connect(m_worker, &KDatabaseWorker::searchRoomsFinished, this, &KCoderWidget::handleSearchRooms);
	connect(this, &KCoderWidget::startShowPersonalReserve, m_worker, &KDatabaseWorker::performPersonalReserveQuery);
	connect(m_worker, &KDatabaseWorker::personalReserveQueryFinished, this, &KCoderWidget::handleShowPersonalReserve);
	connect(this, &KCoderWidget::startConcealReserve, m_worker, &KDatabaseWorker::performPersonalReserveQuery);
	connect(m_worker, &KDatabaseWorker::personalReserveQueryFinishedNeedConceal, this, &KCoderWidget::handleConcealReserve);
	connect(this, &KCoderWidget::startShowAllReserve, m_worker, &KDatabaseWorker::performRoomQuery);
	connect(m_worker, &KDatabaseWorker::roomsQueryFinished, this, &KCoderWidget::handleShowAllReserve);
	m_queryThread.start();
}

KCoderWidget::~KCoderWidget()
{
	delete ui;
	m_queryThread.quit();
	m_queryThread.wait();
}

void KCoderWidget::on_searchRooms_clicked()
{
	QString week = ui->weekDate->currentText();
	QString day = ui->dayDate->currentText();
	emit startSearchRooms(week, day);
}

// 预约会议室：查询指定时间段是否已经被预约，没有才可以被预约
void KCoderWidget::on_reserveRoom_clicked()
{
	QString room = ui->comboBoxRooms->currentText();
	QString week = ui->weekDate->currentText();
	QString day = ui->dayDate->currentText();
	QString res = m_database.reserveRoom(m_userName, room, week, day);
	if (res == "Error")
	{
		QMessageBox::warning(this, "错误", "预约失败，请重试");
	}
	else if (res == "Already")
	{
		QMessageBox::warning(this, "错误", "该时间段内该会议室已经被预约");
	}
	else if (res == "specialSuccess")
	{
		QMessageBox::information(this, "提示", m_userName + "您好，" + room + week + day + "预约成功，等待审批");
		on_showPersonalReserve_clicked();
	}
	else
	{
		QMessageBox::information(this, "提示", m_userName + "您好，" + room + week + day + "预约成功");
	}
}

// 查看个人预约
void KCoderWidget::on_showPersonalReserve_clicked()
{
	ui->reserveResult->clear();
	emit startShowPersonalReserve(m_userName, false);
}

// 取消个人预约
void KCoderWidget::on_concealReserve_clicked()
{
	emit startConcealReserve(m_userName, true);
}

// 查看所有预约信息
void KCoderWidget::on_showAllReserve_clicked()
{
	ui->reserveResult->clear();
	emit startShowAllReserve();
}

// 取消预约
void KCoderWidget::concealReservation(int reservationID)
{
	if (m_database.concealReserve(reservationID))
	{
		QMessageBox::information(this, "提示", "取消预约成功");
		on_showPersonalReserve_clicked();
	}
	else
	{
		QMessageBox::warning(this, "错误", "取消预约失败，请重试");
	}
	on_concealReserve_clicked();
}

// 处理指定时间段会议室状态的查询结果
void KCoderWidget::handleSearchRooms(const QString results, const QString week, const QString day)
{
	if (results == "Error")
	{
		QMessageBox::warning(this, "错误", "查询失败！");
	}
	else
	{
		ui->rooms->clear();
		ui->rooms->setText(week + day + "的会议室情况如下所示：");
		ui->rooms->append(results);
	}
}

// 我的预约查询结果
void KCoderWidget::handleShowPersonalReserve(const QVector<KPersonalReserve> results)
{
	if (results.isEmpty())
	{
		QMessageBox::information(this, "提示", m_userName + "预约信息为空");
	}
	else if (results.first().reserveID == -1)
	{
		QMessageBox::warning(this, "错误", "查询失败，请重试");
	}
	else
	{
		ui->reserveResult->setText(m_userName + "的预约信息如下：");
		for (const KPersonalReserve item : results)
		{
			QString oneRes;
			oneRes += QString("预约ID：%1，会议室：%2，预约时间：%3 %4，预约状态：%5\n")
				.arg(item.reserveID)
				.arg(item.roomName)
				.arg(item.week)
				.arg(item.day)
				.arg(item.status);
			ui->reserveResult->append(oneRes);
		}
	}
}

// 查询到个人预约后取消个人预约
void KCoderWidget::handleConcealReserve(const QVector<KPersonalReserve> results)
{
	if (results.isEmpty())
	{
		QMessageBox::information(this, "提示", m_userName + "预约信息为空");
	}
	else if (results.first().reserveID == -1)
	{
		QMessageBox::warning(this, "错误", "查询失败，请重试");
	}
	else
	{
		KConcealDialog kdialog(results);
		connect(&kdialog, &KConcealDialog::confirmed, this, &KCoderWidget::concealReservation);
		kdialog.exec();
	}
}

// 处理所有预约信息的查询结果
void KCoderWidget::handleShowAllReserve(const QString results)
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
		ui->reserveResult->setText("所有预约信息如下：");
		ui->reserveResult->append(results);
	}
}

void KCoderWidget::on_logout_clicked()
{
	this->close();
	emit logout();
}