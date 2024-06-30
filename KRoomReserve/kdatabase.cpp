#include "kdatabase.h"

KDataBase &KDataBase::getInstance()
{
	static KDataBase instance;
	return instance;
}

// 加密密码
QString KDataBase::encryptPassword(const QString &password)
{
    QCryptographicHash hash(QCryptographicHash::Sha256);    // 选择加密算法
    hash.addData(password.toUtf8());    // 加密密码
    return hash.result().toHex();   // 将得到的哈希值转为16进制的字符串
}

// 建立连接
bool KDataBase::connectDataBase()
{
    static QSqlDatabase database;
    if (!QSqlDatabase::contains("room_reserve_connection"))
    {
        database = QSqlDatabase::addDatabase("QSQLITE", "room_reserve_connection");
        database.setDatabaseName(QCoreApplication::applicationDirPath() + "/KRoomReserve.db");
    }
    else 
    {
        database = QSqlDatabase::database("room_reserve_connection");
    }
    if (database.isOpen())
    {
        qDebug() << "Database already open.";
        return true;
    }
    if (!database.open()) 
    {
        qDebug() << "Error: Connection to database failed" << database.lastError().text();
        return false;
    }
    // 检查SQLite驱动的线程安全级别
    QSqlQuery query(database);
    if (query.exec("PRAGMA compile_options;")) 
    {
        while (query.next()) 
        {
            QString option = query.value(0).toString();
            if (option.contains("THREADSAFE"))
                qDebug() << "SQLite Compile Option:" << option;
        }
    }
    // 设置WAL模式
    if (!query.exec("PRAGMA journal_mode=WAL;"))
        qDebug() << "Failed to set WAL mode:" << query.lastError().text();
    else
        qDebug() << "Set WAL mode success.";

    qDebug() << "Database connected successfully.";
   
    // 增加页面大小，默认是2000
    /*if (!query.exec("PRAGMA cache_size= 10000 ;"))
        qDebug() << "Failed to set cache size:" << query.lastError().text();
    else
        qDebug() << "Set cache size =10000.";*/

    return true;
}

// 断开连接
void KDataBase::disconnectDataBase()
{
    if(QSqlDatabase::contains("room_reserve_connection"))
    {
        QSqlDatabase database = QSqlDatabase::database("room_reserve_connection", false);
        if (database.isOpen())
            database.close();
        QSqlDatabase::removeDatabase("room_reserve_connection");
    }
    else
    {
        qDebug() << "Attempted to disconnect a non-existent or already removed connection.";
    }
}

// 创建数据表，如果已存在则不创建
void KDataBase::createTable() const
{
    QSqlQuery query(QSqlDatabase::database("room_reserve_connection")); // 指定连接
    // 创建Users表，用户表，包含用户ID、用户名、姓名、密码、身份角色
    query.exec("CREATE TABLE IF NOT EXISTS Users ("
        "UserID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "Username TEXT UNIQUE NOT NULL, "
        "Name TEXT NOT NULL, "
        "Password TEXT NOT NULL, "
        "Role TEXT NOT NULL)");

    // 创建MeetingRooms表，会议室表，包含会议室ID、名称，容量，是否需审批
    query.exec("CREATE TABLE IF NOT EXISTS MeetingRooms ("
        "RoomID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "Name TEXT NOT NULL, "
        "Capacity INTEGER NOT NULL, "
        "IsSpecial BOOLEAN NOT NULL DEFAULT FALSE)");

    // 创建Reservations表，预约表，包含预约ID、会议室ID、用户ID、预约周、预约日、预约状态
    // 预约状态包含：预约成功、预约失败、审批中三种
    query.exec("CREATE TABLE IF NOT EXISTS Reservations ("
        "ReservationID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "RoomID INTEGER, "
        "UserID INTEGER, "
        "Weekday TEXT CHECK(Weekday IN ('周一', '周二', '周三', '周四', '周五')) NOT NULL, "
        "TimeOfDay TEXT CHECK(TimeOfDay IN ('上午', '下午')) NOT NULL, "
        "Status TEXT NOT NULL, "
        "FOREIGN KEY(RoomID) REFERENCES MeetingRooms(RoomID), "
        "FOREIGN KEY(UserID) REFERENCES Users(UserID))");

    if (query.lastError().type() != QSqlError::NoError)
        qDebug() << "Error: Failed to create tables." << query.lastError().text();
}

// 判断数据表是否存在
bool KDataBase::isExistTable(const QString &tableName) const
{
    QSqlDatabase database = QSqlDatabase::database("room_reserve_connection");
    if (database.tables().contains(tableName))
        return true;
    return false;
}

// 插入单个用户
bool KDataBase::insertUserData(const KUserInfo &user) const
{
    QSqlQuery query(QSqlDatabase::database("room_reserve_connection")); // 指定连接
    // 开启IMMEDIATE事务
    if (!query.exec("BEGIN IMMEDIATE;"))
    {
        qDebug() << "Error starting IMMEDIATE transaction: " << query.lastError().text();
        return false;
    }
    
    query.prepare("INSERT INTO Users (Username, Name, Password, Role) "
        "VALUES (:username, :name, :password, :role)");
    query.bindValue(":username", user.userName);
    query.bindValue(":name", user.name); // 使用加密过的密码
    query.bindValue(":password", user.password);
    query.bindValue(":role", user.role);
    if (!query.exec())
    {
        qDebug() << "Insert User Error: " << query.lastError();
        query.exec("ROLLBACK;"); // 回滚事务
        return false;
    }
    // 提交事务
    if (!query.exec("COMMIT;"))
    {
        qDebug() << "COMMIT Error: " << query.lastError();
        query.exec("ROLLBACK;"); // 回滚事务
        return false;
    }
    return true;
}

// 插入多个用户
//bool KDataBase::insertManyUsersData(const QList<KUserInfo> &userData) const
//{
//    QSqlQuery sqlQuery(QSqlDatabase::database("room_reserve_connection")); // 指定连接
//    // 写操作的时候开启一个IMMEDIATE事务
//    if (!sqlQuery.exec("BEGIN IMMEDIATE;")) 
//    {
//        qDebug() << "Error starting IMMEDIATE transaction: " << sqlQuery.lastError().text();
//        return false;
//    }
//    // 插入数据的 SQL 语句
//    sqlQuery.prepare("INSERT INTO Users (Username, Name, Password, Role) VALUES (?, ?, ?, ?)");
//    // 插入数据
//    for (const KUserInfo &user : userData)
//    {
//        sqlQuery.addBindValue(user.userName);
//        sqlQuery.addBindValue(user.name);
//        sqlQuery.addBindValue(user.password);
//        sqlQuery.addBindValue(user.role);
//        if (!sqlQuery.exec())
//        {
//            qDebug() << "Failed to insert user:" << user.userName << sqlQuery.lastError().text();
//            sqlQuery.exec("ROLLBACK;"); // 回滚事务 // 如果插入失败，则回滚事务
//            return false;
//        }
//    }
//    // 提交事务
//    if (!sqlQuery.exec("COMMIT;"))
//    {
//        qDebug() << "COMMIT Error: " << sqlQuery.lastError();
//        sqlQuery.exec("ROLLBACK;"); // 回滚事务
//        return false;
//    }
//    return true;
//}

// 初始化会议室信息
bool KDataBase::initMeetingRoom(const QList<KRoomInfo> &roomData) const
{
    QSqlDatabase database = QSqlDatabase::database("room_reserve_connection", false);
    // 开启默认事务
    database.transaction();
    QSqlQuery query(QSqlDatabase::database("room_reserve_connection")); // 指定连接
    query.prepare("INSERT INTO MeetingRooms (Name, Capacity, IsSpecial) VALUES (:name, :capacity, :special)");
    for (const KRoomInfo &room : roomData) 
    {
        query.bindValue(":name", room.roomName);
        query.bindValue(":capacity", room.capacity);
        query.bindValue(":special", room.isSpecial);

        if (!query.exec())
        {
            qDebug() << "Failed to insert meeting room:" << query.lastError();
            database.rollback(); // 如果插入失败，回滚事务
            return false;
        }
    }
    // 提交事务
    database.commit();
    return true;
}

// 查询用户是否存在
bool KDataBase::isExistUser(QString userName) const
{
    QSqlQuery query(QSqlDatabase::database("room_reserve_connection")); // 指定连接
    query.prepare("SELECT COUNT(*)  FROM Users WHERE Username = ?");
    query.addBindValue(userName);
    if (query.exec())
    {
        if (query.next())
            return query.value(0).toInt() > 0;
    }
    qDebug() << "Query failed:" << query.lastError().text();
    return false;
}

// 判断是否存在会议室信息
bool KDataBase::isExistRooms() const
{
    QSqlQuery query(QSqlDatabase::database("room_reserve_connection")); // 指定连接
    query.prepare("SELECT COUNT(*)  FROM MeetingRooms");
    if (query.exec())
    {
        if (query.next())
            return query.value(0).toInt() >= 6;
    }
    qDebug() << "Query failed:" << query.lastError().text();
    return false;
}

// 获取用户身份
QString KDataBase::getUserRole(QString userName) const
{
    QSqlQuery query(QSqlDatabase::database("room_reserve_connection")); // 指定连接
    query.prepare("SELECT Role FROM Users WHERE Username = ?");
    query.addBindValue(userName);
    if (query.exec())
    {
        // 获取并处理查询结果
        query.next();
        QString role = query.value(0).toString(); 
        return role;
    }
    else
    {
        qDebug() << "Query failed:" << query.lastError().text();
        return "Error";
    }
}

// 获取用户密码
QString KDataBase::getUserPassword(QString userName) const
{
    QSqlQuery query(QSqlDatabase::database("room_reserve_connection")); // 指定连接
    query.prepare("SELECT Password FROM Users WHERE Username = ?");
    query.addBindValue(userName);
    if (query.exec())
    {
        // 获取并处理查询结果
        query.next();
        QString password = query.value(0).toString();
        return password;
    }
    else
    {
        qDebug() << "Query failed:" << query.lastError().text();
        return "Error";
    }
}

// 获取所有人员
QString KDataBase::getAllUsers(QString role) const
{
    QSqlQuery query(QSqlDatabase::database("room_reserve_connection")); // 指定连接
    QString statement=QString("SELECT Username, Name, Role FROM Users WHERE Role='%1'").arg(role);
    query.prepare(statement);
    if (!query.exec())
    {
        QString err = "Error";
        qWarning() << "Error:Failed to query database." << query.lastError();
        return err;
    }
    QString users;
    while (query.next())
    {
        QString m_userName = query.value(0).toString();
        QString m_name = query.value(1).toString();
        QString m_role = query.value(2).toString();
        users += QString("用户名：%1，姓名：%2，角色：%3\n").arg(m_userName, m_name, m_role);
    }
    return users;
}

// 获取所有会议室信息，包含预约信息
QString KDataBase::getAllRooms() const
{
    // 定义可能的星期和时间段
    QStringList weekdays = { "周一", "周二", "周三", "周四", "周五" };
    QStringList timeSlots = { "上午", "下午" };
    // 先获取所有会议室的名称
    QMap<QString, QMap<QString, QString>> schedule; // Key: 会议室名字, Value: Map<时间段, 状态>
    QMap<QString, int> capacities;  // 会议室名字 -> 容量
    QMap<QString, bool> specialStatus;  // 会议室名字 -> 是否需要审批
    QMap<QString, QMap<QString,QString>> reserveUserName;  // 会议室名字 -> <时间段，预约人用户名>
    QMap<QString, QMap<QString,QString>> reserveName;  // 会议室名字 -> <时间段，预约人姓名>
    // 查询会议室信息
    QSqlQuery roomQuery("SELECT Name, Capacity, IsSpecial FROM MeetingRooms", QSqlDatabase::database("room_reserve_connection"));
    QString result;
    if (!roomQuery.exec())
    {
        qDebug() << "Query failed to execute:" << roomQuery.lastError();
        result = "Error";
        return result;
    }
    while (roomQuery.next()) 
    {
        QString roomName = roomQuery.value(0).toString();
        int capacity = roomQuery.value(1).toInt();
        bool isSpecial = roomQuery.value(2).toInt();
        capacities[roomName] = capacity;
        specialStatus[roomName] = isSpecial;
        for (const auto &day : weekdays) 
        {
            for (const auto &slot : timeSlots) 
            {
                schedule[roomName][day + " " + slot] = "空闲"; // 默认设置状态为"空闲"
                reserveUserName[roomName][day + " " + slot] = "无"; //  默认设置预约人用户名为“无”
                reserveName[roomName][day + " " + slot] = "无"; //  默认设置预约人姓名为“无”
            }
        }
    }
    // 查询预约信息，并更新状态
    QSqlQuery reservationQuery(QSqlDatabase::database("room_reserve_connection")); // 指定连接
    reservationQuery.prepare(
        "SELECT m.Name, r.Weekday, r.TimeOfDay, r.Status, u.Username, u.Name "
        "FROM MeetingRooms m "
        "LEFT JOIN Reservations r ON m.RoomID = r.RoomID "
        "LEFT JOIN Users u ON r.UserID = u.UserID"
    );
    if (!reservationQuery.exec())
    {
        qDebug() << "Query failed to execute:" << reservationQuery.lastError();
        result = "Error";
        return result;
    }
    while (reservationQuery.next()) 
    {
        QString roomName = reservationQuery.value(0).toString();
        QString weekday = reservationQuery.value(1).toString();
        QString timeOfDay = reservationQuery.value(2).toString();
        QString status = reservationQuery.value(3).toString();
        QString userName = reservationQuery.value(4).toString();
        QString name = reservationQuery.value(5).toString();
        if (!roomName.isEmpty() && !weekday.isEmpty() && !timeOfDay.isEmpty() && !status.isEmpty())
        {
            schedule[roomName][weekday + " " + timeOfDay] = status; // 更新时间段的状态
            reserveUserName[roomName][weekday + " " + timeOfDay] = userName;   // 更新预约人用户名
            reserveName[roomName][weekday + " " + timeOfDay] = name;   // 更新预约人姓名
        }
    }
    // 格式化输出
    for (const auto &room : schedule.keys())
    {
        result += QString("\n会议室：%1, 容量：%2, 是否需审批：%3\n")
            .arg(room)
            .arg(capacities[room])
            .arg(specialStatus[room] ? "是" : "否");
        int i = 1;
        for (const auto &timeSlot : schedule[room].keys())
        {
            result += QString("时间段：%1, 状态：%2, 预约人用户名：%3, 姓名：%4")
                .arg(timeSlot)
                .arg(schedule[room][timeSlot])
                .arg(reserveUserName[room][timeSlot])
                .arg(reserveName[room][timeSlot]);
            if (i % 2 == 0)
                result += '\n';
            else
                result += " / ";
            i++;
        }
    }
    return result;
}

// 获取指定日期的会议室状态，包含预约信息
QString KDataBase::getTheRooms(QString week, QString halfDay) const
{
    QMap<QString, int> capacities;  // 会议室名字 -> 容量
    QMap<QString, bool> specialStatus;  // 会议室名字 -> 是否需要审批
    QMap<QString, QString> reserveStatus;   // 会议室名字 -> 是否空闲
    QSqlQuery roomQuery("SELECT Name, Capacity, IsSpecial FROM MeetingRooms", QSqlDatabase::database("room_reserve_connection"));
    QString result;
    if (!roomQuery.exec())
    {
        qDebug() << "Query failed to execute:" << roomQuery.lastError();
        result = "Error";
        return result;
    }
    while (roomQuery.next())
    {
        QString roomName = roomQuery.value(0).toString();
        int capacity = roomQuery.value(1).toInt();
        bool isSpecial = roomQuery.value(2).toInt();
        capacities[roomName] = capacity;
        specialStatus[roomName] = isSpecial;
        reserveStatus[roomName] = "空闲"; // 默认设置为"空闲"
    }
    // 查询预约信息，并更新状态
    QSqlQuery query(QSqlDatabase::database("room_reserve_connection")); // 指定连接
    query.prepare(
        "SELECT m.Name, r.Status "
        "FROM MeetingRooms m "
        "LEFT JOIN Reservations r ON m.RoomID = r.RoomID "
        "WHERE r.WeekDay = :weekDay AND r.TimeOfDay = :timeOfDay"
    );
    query.bindValue(":weekDay", week);
    query.bindValue(":timeOfDay", halfDay);
    if (!query.exec())
    {
        qDebug() << "Query failed to execute:" << query.lastError();
        result = "Error";
        return result;
    }

    while (query.next())
    {
        QString roomName = query.value(0).toString();
        QString status = query.value(1).toString();
        reserveStatus[roomName] = status;
    }
    // 格式化输出
    for (const auto &room : capacities.keys())
    {
        result += QString("会议室：%1 容量：%2 是否需审批：%3 状态：%4\n")
            .arg(room)
            .arg(capacities[room])
            .arg(specialStatus[room] ? "是" : "否")
            .arg(reserveStatus[room]);
    }
    return result;
}

// 清除所有预约信息
bool KDataBase::clearReserve() const
{
    QSqlDatabase db = QSqlDatabase::database("room_reserve_connection", false);
    QSqlQuery query(db);
    // 开始IMMEDIATE事务
    if (!query.exec("BEGIN IMMEDIATE;")) {
        qDebug() << "Error starting IMMEDIATE transaction:" << query.lastError().text();
        return false;
    }
    // 清空Reservations表
    if (!query.exec("DELETE FROM Reservations"))
    {
        qDebug() << "Error: Failed to delete from Reservations." << query.lastError();
        query.exec("ROLLBACK;"); // 回滚事务
        return false;
    }
    // 提交事务
    if (!query.exec("COMMIT;"))
    {
        qDebug() << "COMMIT Error: " << query.lastError();
        query.exec("ROLLBACK;"); // 回滚事务
        return false;
    }
    return true;
}

// 预约会议室
QString KDataBase::reserveRoom(QString userName, QString roomName, QString week, QString day)
{
    QSqlQuery query(QSqlDatabase::database("room_reserve_connection")); // 指定连接
    // 获取用户ID
    int userID = -1; // 初始值为无效ID
    query.prepare("SELECT UserID FROM Users WHERE Username = :UserName");
    query.bindValue(":UserName", userName);
    if (query.exec())
    {
        if (query.next()) 
            userID = query.value(0).toInt();
    }
    else
    {
        qDebug() << "查询用户ID失败: " << query.lastError().text();
        return "Error";
    }

    // 查询会议室ID 和是否需要审批
    query.prepare("SELECT RoomID, IsSpecial FROM MeetingRooms WHERE Name = :RoomName");
    query.bindValue(":RoomName", roomName);
    int roomID = -1; // 初始值为无效ID
    bool isSpecial = false; // 默认无需审批
    if (query.exec())
    {
        if (query.next())
        {
            roomID = query.value(0).toInt(); // 获取会议室ID
            isSpecial = query.value(1).toInt(); // 是否需要审批
        }
    }
    else
    {
        qDebug() << "查询会议室ID失败: " << query.lastError().text();
        return "Error";
    }

    // 检查会议室是否可用
    query.prepare("SELECT Status "
        "FROM Reservations "
        "WHERE RoomID = :RoomID AND Weekday = :Weekday AND TimeOfDay = :TimeOfDay");
    query.bindValue(":RoomID", roomID);
    query.bindValue(":Weekday", week);
    query.bindValue(":TimeOfDay", day);

    if (!query.exec())
    {
        qDebug() << "查询预约信息失败: " << query.lastError().text();
        return "Error";
    }

    // 如果已经存在预约信息，并且为审批中或者预约成功，则表示不能继续预约
    while (query.next())
    {
        if (query.value(0).toString() == "预约成功" || query.value(0).toString() == "审批中")
            return "Already";
    }
        
    // 如果会议室可以被预约，则插入新的预约记录
    QString newStatus = isSpecial ? "审批中" : "预约成功";
    // 开始IMMEDIATE事务
    if (!query.exec("BEGIN IMMEDIATE;")) {
        qDebug() << "Error starting IMMEDIATE transaction:" << query.lastError().text();
        return "Error";
    }
    query.prepare("INSERT INTO Reservations (RoomID, UserID, Weekday, TimeOfDay, Status) "
        "VALUES (:RoomID, :UserID, :Weekday, :TimeOfDay, :Status)");
    query.bindValue(":RoomID", roomID);
    query.bindValue(":UserID", userID);
    query.bindValue(":Weekday", week);
    query.bindValue(":TimeOfDay", day);
    query.bindValue(":Status", newStatus);
    if (!query.exec())
    {
        qDebug() << userName + "预约失败: " << query.lastError().text();
        query.exec("ROLLBACK;");
        return "Error";
    }
    // 提交事务
    if (!query.exec("COMMIT;"))
    {
        qDebug() << "COMMIT Error: " << query.lastError();
        query.exec("ROLLBACK;"); // 回滚事务
        return false;
    }
    qDebug() << userName + "预约成功";
    if (isSpecial) 
        return "specialSuccess";
    return "Success";
}

// 查询用户的预约信息
QVector<KPersonalReserve> KDataBase::getPersonalReserve(QString userName)
{
    QVector<KPersonalReserve> res;
    QSqlQuery query(QSqlDatabase::database("room_reserve_connection")); // 指定连接
    // 获取用户ID
    qint32 userID = -1; // 初始值为无效ID
    query.prepare("SELECT UserID FROM Users WHERE Username = :UserName");
    query.bindValue(":UserName", userName);
    if (query.exec())
    {
        if (query.next())
            userID = query.value(0).toInt();
    }
    else
    {
        qDebug() << "查询用户ID失败: " << query.lastError().text();
        KPersonalReserve error = { -1,"Error","Error","Error","Error" };
        res.push_back(error);
        return res;
    }
    // 检查是否有预约信息
    // 使用用户ID查询该用户的所有预约信息
    query.prepare("SELECT r.ReservationID, r.Weekday, r.TimeOfDay, r.Status, m.Name "
        "FROM Reservations r INNER JOIN MeetingRooms m ON r.RoomID = m.RoomID "
        "WHERE r.UserID = :UserID");
    query.bindValue(":UserID", userID);
    if (query.exec())
    {
        while (query.next())
        {
            int reservationID = query.value(0).toInt(); // 获取预约ID
            QString weekday = query.value(1).toString();
            QString timeOfDay = query.value(2).toString();
            QString status = query.value(3).toString();
            QString roomName = query.value(4).toString();
            KPersonalReserve tmp = { reservationID,roomName,weekday,timeOfDay,status };
            res.push_back(tmp);
        }
        return res;
    }
    else 
    {
        KPersonalReserve error = { -1,"Error","Error","Error","Error" };
        res.push_back(error);
        return res;
    }
}

// 取消预约
bool KDataBase::concealReserve(int reservationID)
{
    QSqlQuery query(QSqlDatabase::database("room_reserve_connection")); // 指定连接
    // 开始IMMEDIATE事务
    if (!query.exec("BEGIN IMMEDIATE;")) {
        qDebug() << "Error starting IMMEDIATE transaction:" << query.lastError().text();
        return false;
    }
    query.prepare("UPDATE Reservations SET Status = '预约取消' WHERE ReservationID = :ReservationID");
    query.bindValue(":ReservationID", reservationID);
    if (!query.exec())
    {
        qDebug() << reservationID+"取消预约失败:" << query.lastError().text();
        query.exec("ROLLBACK;");
        return false;
    }
    // 提交事务
    if (!query.exec("COMMIT;"))
    {
        qDebug() << "COMMIT Error: " << query.lastError();
        query.exec("ROLLBACK;"); // 回滚事务
        return false;
    }
    qDebug() << "预约已取消";
    return true;
}

// 获取待审批的预约信息
QVector<KWaitApproval> KDataBase::getWaitApprovalReserve() const
{
    QVector<KWaitApproval> res;
    QSqlQuery query(QSqlDatabase::database("room_reserve_connection")); // 指定连接
    query.prepare("SELECT r.ReservationID, r.Weekday, r.TimeOfDay, r.Status, m.Name AS RoomName, u.Username, u.Name "
        "FROM Reservations r "
        "INNER JOIN MeetingRooms m ON r.RoomID = m.RoomID "
        "INNER JOIN Users u ON r.UserID = u.UserID "
        "WHERE r.Status = '审批中'");
    if (query.exec())
    {
        while (query.next())
        {
            int reservationID = query.value(0).toInt(); // 获取预约ID
            QString weekday = query.value(1).toString();
            QString timeOfDay = query.value(2).toString();
            QString status = query.value(3).toString();
            QString roomName = query.value(4).toString();
            QString userName = query.value(5).toString();
            QString name = query.value(6).toString();
            KWaitApproval tmp = { reservationID, userName, name, roomName, weekday, timeOfDay, status };
            res.push_back(tmp);
        }
        return res;
    }
    else
    {
        KWaitApproval error = { -1,"Error","Error","Error","Error", "Error", "Error" };
        res.push_back(error);
        return res;
    }
}

// 审批预约
bool KDataBase::approvalReservation(qint32 reservationID, bool isPassed)
{
    QSqlQuery query(QSqlDatabase::database("room_reserve_connection")); // 指定连接
    // 开始IMMEDIATE事务
    if (!query.exec("BEGIN IMMEDIATE;")) {
        qDebug() << "Error starting IMMEDIATE transaction:" << query.lastError().text();
        return false;
    }
    if (isPassed) 
    {
        // 更新预约状态为“预约成功”
        query.prepare("UPDATE Reservations SET Status = '预约成功' WHERE ReservationID = :ReservationID");
        query.bindValue(":ReservationID", reservationID);
        if (!query.exec()) 
        {
            qDebug() << "更新预约状态失败:" << query.lastError().text();
            query.exec("ROLLBACK;");
            return false;
        }
    }
    else
    {
        // 更新预约状态为“预约失败”
        query.prepare("UPDATE Reservations SET Status = '预约失败' WHERE ReservationID = :ReservationID");
        query.bindValue(":ReservationID", reservationID);
        if (!query.exec())
        {
            qDebug() << "驳回预约失败:" << query.lastError().text();
            query.exec("ROLLBACK;");
            return false;
        }
    }
    // 提交事务
    if (!query.exec("COMMIT;"))
    {
        qDebug() << "COMMIT Error: " << query.lastError();
        query.exec("ROLLBACK;"); // 回滚事务
        return false;
    }
    return true;
}
