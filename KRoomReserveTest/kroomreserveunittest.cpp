#include "kroomreserveunittest.h"

KRoomReserveUnitTest::KRoomReserveUnitTest()
	: m_db(KDataBase::getInstance())
	, roomReserve(nullptr)
	, adminWidget(nullptr)
	, coderWidget(nullptr)
	, approvalWidget(nullptr)
{
}

void KRoomReserveUnitTest::initTestCase()
{
	m_db.connectDataBase();	// 建立数据库连接
}

// 测试数据库的初始化操作
void KRoomReserveUnitTest::initDBTest()
{
	QVERIFY(m_db.connectDataBase());
	m_db.createTable();
	QVERIFY(m_db.isExistTable("Users"));
	QVERIFY(m_db.isExistTable("MeetingRooms"));
	QVERIFY(m_db.isExistTable("Reservations"));
	// 创建会议室信息
	QList<KRoomInfo> rooms;
	rooms << KRoomInfo{ u8"紫阳湖会议室",14,true };
	rooms << KRoomInfo{ u8"后官湖会议室",8,false };
	rooms << KRoomInfo{ u8"清潭湖会议室",10,false };
	rooms << KRoomInfo{ u8"黄家湖会议室",6,false };
	rooms << KRoomInfo{ u8"杨春湖会议室",18,true };
	rooms << KRoomInfo{ u8"中山湖会议室",150,true };
	QVERIFY(m_db.initMeetingRoom(rooms));
	m_db.disconnectDataBase();
	QSqlDatabase database = QSqlDatabase::database("room_reserve_connection", false);
	QVERIFY(!database.isOpen());
}

// 测试登录功能
void KRoomReserveUnitTest::loginUnitTest()
{
	roomReserve = new KRoomReserve();
	// 输入用户名和密码
	roomReserve->ui->userName->clear();
	roomReserve->ui->password->clear();
	QTest::keyClicks(roomReserve->ui->userName, "admin");
	QTest::keyClicks(roomReserve->ui->password, "admin");
	// 选择角色
	roomReserve->ui->comboBoxIdentity->setCurrentIndex(2);
	QSignalSpy spy(roomReserve->ui->loginButton, &QPushButton::clicked);
	QTest::mouseClick(roomReserve->ui->loginButton, Qt::LeftButton);
	QCOMPARE(spy.count(), 1);
	QString userName = roomReserve->ui->userName->text();
	QString password = roomReserve->ui->password->text();
	QString role = roomReserve->ui->comboBoxIdentity->currentText();
	QCOMPARE(roomReserve->m_database.isExistUser(userName), true);
	QCOMPARE(roomReserve->m_database.getUserPassword(userName), roomReserve->m_database.encryptPassword(password));
	QCOMPARE(roomReserve->m_database.getUserRole(userName), role);
}

// 测试添加人员
void KRoomReserveUnitTest::addPeopleTest()
{
	adminWidget = new KAdminWidget();
	QTest::keyClicks(adminWidget->ui->userName, "UnitTest");
	QTest::keyClicks(adminWidget->ui->name, "UnitTest");
	QTest::keyClicks(adminWidget->ui->password, "UnitTest");
	QTest::keyClicks(adminWidget->ui->password_2, "UnitTest");
	adminWidget->ui->comboBoxIdentity->setCurrentIndex(1);	// 设置为研发人员
	QString userName = adminWidget->ui->userName->text();
	QString name = adminWidget->ui->name->text();
	QString password = adminWidget->ui->password->text();
	QString password_2 = adminWidget->ui->password_2->text();
	QString role = adminWidget->ui->comboBoxIdentity->currentText();
	QSignalSpy spy(adminWidget->ui->addPeople, &QPushButton::clicked);
	QTest::mouseClick(adminWidget->ui->addPeople, Qt::LeftButton);
	QCOMPARE(spy.count(), 1);
	if (!m_db.isExistUser(userName))
	{
		KUserInfo user = { userName,name,m_db.encryptPassword(password),role };
		QCOMPARE(m_db.insertUserData(user), true);
	}
	
	userName = "UnitTest_2";
	name = "UnitTest_2";
	password = "UnitTest_2";
	password_2 = "UnitTest_2";
	role = u8"研发人员";
	if (!m_db.isExistUser(userName))
	{
		KUserInfo user = { userName,name,m_db.encryptPassword(password),role };
		QCOMPARE(m_db.insertUserData(user), true);
	}
	userName = "UnitTest_3";
	name = "UnitTest_3";
	password = "UnitTest_3";
	password_2 = "UnitTest_3";
	role = u8"行政人员";
	if (!m_db.isExistUser(userName))
	{
		KUserInfo user = { userName,name,m_db.encryptPassword(password),role };
		QCOMPARE(m_db.insertUserData(user), true);
	}
}

// 测试获取人员
void KRoomReserveUnitTest::getPeopleTest()
{
	QSignalSpy spy_1(adminWidget->ui->getCoders, &QPushButton::clicked);
	QSignalSpy spy_2(adminWidget->ui->getAdmins, &QPushButton::clicked);
	QTest::mouseClick(adminWidget->ui->getCoders, Qt::LeftButton);
	QTest::mouseClick(adminWidget->ui->getAdmins, Qt::LeftButton);
	QCOMPARE(spy_1.count(), 1);
	QCOMPARE(spy_2.count(), 1);
	QVERIFY(m_db.getAllUsers("研发人员") != "Error");
	QVERIFY(m_db.getAllUsers("行政人员") != "Error");
}

// 测试获取所有会议室信息
void KRoomReserveUnitTest::getMeetingRoomsTest()
{
	QSignalSpy spy(adminWidget->ui->showRooms, &QPushButton::clicked);
	QTest::mouseClick(adminWidget->ui->showRooms, Qt::LeftButton);
	QCOMPARE(spy.count(), 1);
	QVERIFY(m_db.getAllRooms() != "Error");
}

// 测试清空所有预约信息
void KRoomReserveUnitTest::clearAllReserveTest()
{
	QSignalSpy spy(adminWidget->ui->clearAllReserve, &QPushButton::clicked);
	QTest::mouseClick(adminWidget->ui->clearAllReserve, Qt::LeftButton);
	QCOMPARE(spy.count(), 1);
	QVERIFY(m_db.clearReserve());
}

// 测试研发人员查看指定日期的会议室信息
void KRoomReserveUnitTest::getThatRoomsTest()
{
	coderWidget = new KCoderWidget("UnitTest");
	coderWidget->ui->weekDate->setCurrentIndex(2);	// 选择周三
	coderWidget->ui->dayDate->setCurrentIndex(0);	// 选择上午
	QSignalSpy spy(coderWidget->ui->searchRooms, &QPushButton::clicked);
	QTest::mouseClick(coderWidget->ui->searchRooms, Qt::LeftButton);
	QCOMPARE(spy.count(), 1);
	QString week = coderWidget->ui->weekDate->currentText();
	QString day = coderWidget->ui->dayDate->currentText();
	QVERIFY(m_db.getTheRooms(week, day) != "Error");
}

// 测试研发人员预约会议室
void KRoomReserveUnitTest::reserveMeetingRoomTest()
{
	coderWidget->ui->comboBoxRooms->setCurrentIndex(0);	// 选择紫阳湖会议室
	QString room = coderWidget->ui->comboBoxRooms->currentText();
	QString week = coderWidget->ui->weekDate->currentText();	// 周三
	QString day = coderWidget->ui->dayDate->currentText();	// 上午
	QSignalSpy spy(coderWidget->ui->reserveRoom, &QPushButton::clicked);
	QTest::mouseClick(coderWidget->ui->reserveRoom, Qt::LeftButton);
	QCOMPARE(spy.count(), 1);
	QVERIFY(m_db.reserveRoom("UnitTest",room,week, day) != "Error");
	room = u8"后官湖会议室";
	week = u8"周五";
	day = u8"下午";
	QVERIFY(m_db.reserveRoom("UnitTest_2",room,week, day) != "Error");
	room = u8"中山湖会议室";
	week = u8"周一";
	day = u8"上午";
	QVERIFY(m_db.reserveRoom("UnitTest_3", room, week, day) != "Error");
}

// 测试研发人员查看个人预约
void KRoomReserveUnitTest::showPersonalReserveTest()
{
	QSignalSpy spy(coderWidget->ui->showPersonalReserve, &QPushButton::clicked);
	QTest::mouseClick(coderWidget->ui->showPersonalReserve, Qt::LeftButton);
	QCOMPARE(spy.count(), 1);
	QVector<KPersonalReserve> res = m_db.getPersonalReserve("UnitTest");
	KPersonalReserve item = res.back();
	QCOMPARE(item.roomName, QString(u8"紫阳湖会议室"));
	QCOMPARE(item.week, QString(u8"周三"));
	QCOMPARE(item.day, QString(u8"上午"));
	res.clear();
	res = m_db.getPersonalReserve("UnitTest_2");
	item = res.back();
	QCOMPARE(item.roomName, QString(u8"后官湖会议室"));
	QCOMPARE(item.week, QString(u8"周五"));
	QCOMPARE(item.day, QString(u8"下午"));
}

// 测试研发人员查看所有预约
void KRoomReserveUnitTest::showAllReserveTest()
{
	QSignalSpy spy(coderWidget->ui->showAllReserve, &QPushButton::clicked);
	QTest::mouseClick(coderWidget->ui->showAllReserve, Qt::LeftButton);
	QCOMPARE(spy.count(), 1);
}

// 测试研发人员取消个人预约
void KRoomReserveUnitTest::concealPersonalReserveTest()
{
	QSignalSpy spy(coderWidget->ui->concealReserve, &QPushButton::clicked);
	QTest::mouseClick(coderWidget->ui->concealReserve, Qt::LeftButton);
	QCOMPARE(spy.count(), 1);
	QVector<KPersonalReserve> res = m_db.getPersonalReserve("UnitTest_2");
	const KPersonalReserve item = res.back();
	QVERIFY(m_db.concealReserve(item.reserveID));
}

// 测试研发人员取消预约的对话框
void KRoomReserveUnitTest::concealReserveDialogTest()
{
	QVector<KPersonalReserve> res = m_db.getPersonalReserve("UnitTest");
	KConcealDialog concealDialog(res);
	QSignalSpy spy(concealDialog.ui->confirmButton, &QPushButton::clicked);
	QTest::mouseClick(concealDialog.ui->confirmButton, Qt::LeftButton);
	QCOMPARE(spy.count(), 1);
	QSignalSpy spy_2(concealDialog.ui->cancelButton, &QPushButton::clicked);
	QTest::mouseClick(concealDialog.ui->cancelButton, Qt::LeftButton);
	QCOMPARE(spy_2.count(), 1);
}

// 测试行政人员查看所有待审批的信息
void KRoomReserveUnitTest::showApprovalReserveTest()
{
	approvalWidget = new KApprovalWidget();
	QSignalSpy spy(approvalWidget->ui->showApprovalReserve, &QPushButton::clicked);
	QTest::mouseClick(approvalWidget->ui->showApprovalReserve, Qt::LeftButton);
	QCOMPARE(spy.count(), 1);
	QVector<KWaitApproval> res = m_db.getWaitApprovalReserve();
	QVERIFY(!res.isEmpty());
}

// 测试行政人员审批预约
void KRoomReserveUnitTest::approvalReserveTest()
{
	QSignalSpy spy(approvalWidget->ui->agreeReserve, &QPushButton::clicked);
	QTest::mouseClick(approvalWidget->ui->agreeReserve, Qt::LeftButton);
	QCOMPARE(spy.count(), 1);
	QSignalSpy spy_2(approvalWidget->ui->refuseReserve, &QPushButton::clicked);
	QTest::mouseClick(approvalWidget->ui->refuseReserve, Qt::LeftButton);
	QCOMPARE(spy_2.count(), 1);
	QVector<KWaitApproval> res = m_db.getWaitApprovalReserve();
	KWaitApproval item = res.front();
	QVERIFY(m_db.approvalReservation(item.reserveID, true));
	item = res.back();
	QVERIFY(m_db.approvalReservation(item.reserveID, false));
}

// 注销登录测试
void KRoomReserveUnitTest::logoutTest()
{
	QSignalSpy spy(approvalWidget->ui->logout, &QPushButton::clicked);
	QTest::mouseClick(approvalWidget->ui->logout, Qt::LeftButton);
	QCOMPARE(spy.count(), 1);
	QSignalSpy spy_2(adminWidget->ui->logoutButton, &QPushButton::clicked);
	QTest::mouseClick(adminWidget->ui->logoutButton, Qt::LeftButton);
	QCOMPARE(spy_2.count(), 1);
	QSignalSpy spy_3(coderWidget->ui->logout, &QPushButton::clicked);
	QTest::mouseClick(coderWidget->ui->logout, Qt::LeftButton);
	QCOMPARE(spy_3.count(), 1);
}

void KRoomReserveUnitTest::cleanupTestCase()
{
	m_db.disconnectDataBase();	// 关闭数据库连接
	if (roomReserve)
	{
		delete roomReserve;
		roomReserve = nullptr;
	}
	if (adminWidget)
	{
		delete adminWidget;
		adminWidget = nullptr;
	}
	if (coderWidget)
	{
		delete coderWidget;
		coderWidget = nullptr;
	}
	if (approvalWidget)
	{
		delete approvalWidget;
		approvalWidget = nullptr;
	}
}
