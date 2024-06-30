// -----------------------------------------------------------
// kdatabase.h
// 创建者： 李康
// 创建时间： 2024/4/20
// 功能描述： 会议室预约系统——数据库操作相关功能
// -----------------------------------------------------------

#ifndef _KROOMRESERVE_KDATABASE_H_
#define _KROOMRESERVE_KDATABASE_H_

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QCoreApplication>
#include <QDebug>
#include <QVector>
#include <QCryptographicHash>

struct KUserInfo	// 人员表信息
{
	QString userName;	// 用户名
	QString name;	// 姓名
	QString password;	// 密码
	QString role;	// 研发人员，行政人员，系统管理员
};

struct KRoomInfo	// 会议室表信息
{
	QString roomName;	// 会议室名称
	int capacity;	// 容量
	bool isSpecial;	// 是否为特殊会议室
};

struct KPersonalReserve	// 查询个人的预约信息用
{
	int reserveID;	// 预约id
	QString roomName;	// 预约的会议室名称
	QString week;	// 预约的星期
	QString day;	// 预约的具体时间段-上午/下午
	QString status;	// 预约的状态-审批中/预约成功
};

struct KWaitApproval	// 查询所有待审批的信息用
{
	int reserveID;	// 预约id
	QString userName;	// 预约人用户名
	QString name;	// 预约人姓名
	QString roomName;	// 预约的会议室名称
	QString week;	// 预约的星期
	QString day;	// 预约的具体时间段-上午/下午
	QString status;	// 预约的状态
};

// 数据库连接管理类 单例模式
class KDataBase
{
public:
	static KDataBase &getInstance();
	QString encryptPassword(const QString &password);   // 加密密码
	bool connectDataBase();	// 连接数据库
	void disconnectDataBase();	// 断开连接
	void createTable() const;	// 创建数据表
	bool isExistTable(const QString &tableName) const;	// 判断某个表是否存在
	bool insertUserData(const KUserInfo &user) const;	// 插入单个用户
	//bool insertManyUsersData(const QList<KUserInfo> &userData) const;	// 插入批量用户
	bool initMeetingRoom(const QList<KRoomInfo> &roomData) const;	// 初始化会议室信息
	bool isExistUser(QString userName) const;	// 判断某用户是否存在
	bool isExistRooms() const;	// 判断是否存在会议室信息
	QString getUserRole(QString userName) const;	// 获取用户身份
	QString getUserPassword(QString userName) const;	// 获取用户密码
	QString getAllUsers(QString role) const;	// 获取所有人员
	QString getAllRooms() const;	// 获取所有会议室信息
	QString getTheRooms(QString week, QString halfDay) const;
	bool clearReserve() const;	// 清空会议室预约信息
	QString reserveRoom(QString userName, QString roomName, QString week, QString day);	// 预约会议室
	QVector<KPersonalReserve> getPersonalReserve(QString userName);	// 获取个人的预约信息
	bool concealReserve(int reservationID);	// 取消预约
	QVector<KWaitApproval> getWaitApprovalReserve() const;	// 获取待审批的预约信息
	bool approvalReservation(qint32 reservationID, bool isPassed);	// 审批预约

private:
	KDataBase() = default;
	KDataBase(const KDataBase &other) = delete;
	KDataBase &operator=(const KDataBase &other) = delete;
};

#endif // _KROOMRESERVE_KDATABASE_H_
