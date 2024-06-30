// -----------------------------------------------------------
// kroomreserveunittest.h
// 创建者： 李康
// 创建时间： 2024/4/20
// 功能描述： 会议室预约系统——单元测试类
// -----------------------------------------------------------

#ifndef _KROOMRESERVETEST_KROOMRESERVETEST_H_
#define _KROOMRESERVETEST_KROOMRESERVETEST_H_

#include <QObject>
#include <QTest>
#include <QSignalSpy>

#include "kroomreserve.h"

class KRoomReserveUnitTest : public QObject
{
	Q_OBJECT

public:
	KRoomReserveUnitTest();

private slots:
	void initTestCase();	// 初始化操作
	void initDBTest();	// 测试数据库的初始化操作
	void loginUnitTest();	// 测试登录
	void addPeopleTest();	// 测试系统管理员添加研发人员/行政人员
	void getPeopleTest();	// 测试获取人员信息
	void getMeetingRoomsTest();	// 测试获取所有会议室信息
	void clearAllReserveTest();	// 测试清除所有预约信息
	void getThatRoomsTest();	// 测试研发人员查看指定日期的会议室信息
	void reserveMeetingRoomTest();	// 测试研发人员预约会议室
	void showPersonalReserveTest();	// 测试研发人员查看个人预约
	void showAllReserveTest();	// 测试研发人员查看所有预约
	void concealPersonalReserveTest();	// 测试研发人员取消个人预约
	void concealReserveDialogTest();	// 测试研发人员取消预约的对话框
	void showApprovalReserveTest();	// 测试行政人员查看所有待审批的信息
	void approvalReserveTest();	// 测试行政人员审批预约
	void logoutTest();	// 注销登录测试
	void cleanupTestCase();	// 清理资源

private:
	KDataBase &m_db;
	KRoomReserve *roomReserve;
	KAdminWidget *adminWidget;
	KCoderWidget *coderWidget;
	KApprovalWidget *approvalWidget;
};

#endif // _KROOMRESERVETEST_KROOMRESERVETEST_H_
