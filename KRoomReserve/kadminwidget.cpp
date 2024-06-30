#include "kadminwidget.h"

KAdminWidget::KAdminWidget(QWidget *parent)
	: QWidget(parent)
	,m_database(KDataBase::getInstance())
	,ui(new Ui::KAdminWidget())
	,m_worker(new KDatabaseWorker())
{
	ui->setupUi(this);
	m_worker->moveToThread(&m_queryThread);
	connect(&m_queryThread, &QThread::finished, m_worker, &QObject::deleteLater);
	connect(this, &KAdminWidget::startRoomQuery, m_worker, &KDatabaseWorker::performRoomQuery);
	connect(m_worker, &KDatabaseWorker::roomsQueryFinished, this, &KAdminWidget::handleRoomQueryResults);
	connect(this, &KAdminWidget::startCoderQuery, m_worker, &KDatabaseWorker::performCoderQuery);
	connect(m_worker, &KDatabaseWorker::coderQueryFinished, this, &KAdminWidget::handleCoderQueryResults);
	connect(this, &KAdminWidget::startAdminsQuery, m_worker, &KDatabaseWorker::performAdminQuery);
	connect(m_worker, &KDatabaseWorker::adminsQueryFinished, this, &KAdminWidget::handleAdminsQueryResults);
	m_queryThread.start();
}

KAdminWidget::~KAdminWidget()
{
	delete ui;
	m_queryThread.quit();
	m_queryThread.wait();
}

void KAdminWidget::on_addPeople_clicked()
{
	// 先检查数据格式，各项不为空，且输入的两次密码一致
	QString userName = ui->userName->text();
	QString name = ui->name->text();
	QString password = ui->password->text();
	QString password_2 = ui->password_2->text();
	QString role = ui->comboBoxIdentity->currentText();
	if (userName.trimmed().isEmpty())
	{
		QMessageBox::warning(this, "警告", "用户名不能为空白字符或者为空");
		return;
	}
	if (name.trimmed().isEmpty())
	{
		QMessageBox::warning(this, "警告", "姓名不能为空白字符或者为空");
		return;
	}
	if (password.trimmed().isEmpty())
	{
		QMessageBox::warning(this, "警告", "密码不能为空白字符或者为空");
		return;
	}
	if (password != password_2)
	{
		QMessageBox::warning(this, "警告", "两次输入的密码不一致");
		return;
	}
	if (m_database.isExistUser(userName))
	{
		QMessageBox::warning(this, "警告", "用户名已存在");
		return;
	}
	// 加密密码
	password = m_database.encryptPassword(password_2);
	KUserInfo user = { userName,name,password,role };
	if (m_database.insertUserData(user))
		QMessageBox::information(this, "提示", role+userName + "添加成功");
	else
		QMessageBox::information(this, "错误", role+userName + "添加失败");
}

// 获取所有研发人员信息
void KAdminWidget::on_getCoders_clicked()
{
	emit startCoderQuery();
}

// 获取所有行政人员信息
void KAdminWidget::on_getAdmins_clicked()
{
	emit startAdminsQuery();
}

// 查询所有会议室信息
void KAdminWidget::on_showRooms_clicked()
{
	emit startRoomQuery();
}

// 清空所有预约信息
void KAdminWidget::on_clearAllReserve_clicked()
{
	if (m_database.clearReserve())
		QMessageBox::information(this, "提示", "预约信息已清空");
	else
		QMessageBox::warning(this, "错误", "预约信息清理失败");
}

// 处理会议室信息的查询结果
void KAdminWidget::handleRoomQueryResults(const QString results)
{
	if (results =="Error")
	{
		QMessageBox::warning(this, "错误", "查询失败！请重试");
	}
	else if (results.isEmpty())
	{
		QMessageBox::information(this, "提示", "暂无会议室信息");
	}
	else
	{
		ui->textBrowserRooms->clear();
		ui->textBrowserRooms->setText("所有会议室信息如下：");
		ui->textBrowserRooms->append(results);
	}
}

// 处理研发人员信息的查询结果
void KAdminWidget::handleCoderQueryResults(const QString results)
{
	if (results == "Error")
	{
		QMessageBox::warning(this, "错误", "查询失败！请重试");
	}
	else if (results.isEmpty())
	{
		QMessageBox::information(this, "提示", "暂无研发人员信息");
	}
	else
	{
		ui->textBrowserUsers->clear();
		ui->textBrowserUsers->setText("所有研发人员信息如下：");
		ui->textBrowserUsers->append(results);
	}
}

// 处理行政人员查询结果
void KAdminWidget::handleAdminsQueryResults(const QString results)
{
	if (results == "Error")
	{
		QMessageBox::warning(this, "错误", "查询失败！请重试");
	}
	else if (results.isEmpty())
	{
		QMessageBox::information(this, "提示", "暂无行政人员信息");
	}
	else
	{
		ui->textBrowserUsers->clear();
		ui->textBrowserUsers->setText("所有行政人员信息如下：");
		ui->textBrowserUsers->append(results);
	}
}

void KAdminWidget::on_logoutButton_clicked()
{
	this->close();
	emit logout();
}
