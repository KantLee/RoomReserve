#include "kroomreserve.h"

KRoomReserve::KRoomReserve(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::KRoomReserveClass())
    ,m_database(KDataBase::getInstance())
    ,m_adminWindow(nullptr)
    ,m_approvalWindow(nullptr)
    ,m_coderWindow(nullptr)
{
    ui->setupUi(this);
    // 加上这句，否则QWidget不显示背景
    setAttribute(Qt::WA_StyledBackground, true);
    initDataBase();
}

KRoomReserve::~KRoomReserve()
{
    delete ui;
    m_database.disconnectDataBase();
    if (m_adminWindow)
    {
        delete m_adminWindow;
        m_adminWindow = nullptr;
    }
    if (m_approvalWindow)
    {
        delete m_approvalWindow;
        m_approvalWindow = nullptr;
    }
    if (m_coderWindow)
    {
        delete m_coderWindow;
        m_coderWindow = nullptr;
    }
}

void KRoomReserve::initDataBase()
{
    if (!m_database.connectDataBase())
    {
        QMessageBox::warning(this, "错误", "数据库连接失败");
        return;
    }
    // 创建数据表
    m_database.createTable();

    // 默认的管理员账户是admin
    if (!m_database.isExistUser("admin"))
    {
        QString adminPassword = m_database.encryptPassword("admin");    // 加密密码
        KUserInfo admin = { "admin","超级管理员", adminPassword, "系统管理员" };
        if (!m_database.insertUserData(admin))
            QMessageBox::warning(this, "错误", "管理员初始化失败");
    }
    // 创建会议室信息
    QList<KRoomInfo> rooms;
    rooms << KRoomInfo{ "会议室001",14,true };
    rooms << KRoomInfo{ "会议室002",8,false };
    rooms << KRoomInfo{ "会议室003",10,false };
    rooms << KRoomInfo{ "会议室004",6,false  };
    rooms << KRoomInfo{ "会议室005",18,true };
    rooms << KRoomInfo{ "会议室006",150,true };
    if (!m_database.isExistRooms())
    {
        if (!m_database.initMeetingRoom(rooms))
            QMessageBox::warning(this, "错误", "会议室初始化失败");
    }
}

void KRoomReserve::on_exitButton_clicked()
{
    close();
}

void KRoomReserve::on_loginButton_clicked()
{
    QString role = ui->comboBoxIdentity->currentText();
    QString userName = ui->userName->text();
    QString password = ui->password->text();
    if (m_database.isExistUser(userName))
    {
        if (m_database.getUserRole(userName) == role)
        {
            if (m_database.getUserPassword(userName) == m_database.encryptPassword(password))
            {
                if (role == "研发人员")
                {
                    m_coderWindow = new KCoderWidget(userName);
                    connect(m_coderWindow, &KCoderWidget::logout, this, &KRoomReserve::show);
                    m_coderWindow->show();
                    this->hide();
                }
                else if (role == "行政人员")
                {
                    m_approvalWindow = new KApprovalWidget();
                    connect(m_approvalWindow, &KApprovalWidget::logout, this, &KRoomReserve::show);
                    m_approvalWindow->show();
                    this->hide();
                }
                else
                {
                    m_adminWindow = new KAdminWidget();
                    connect(m_adminWindow, &KAdminWidget::logout, this, &KRoomReserve::show);
                    m_adminWindow->show();
                    this->hide();
                }
            }
            else
            {
                QMessageBox::warning(this, "错误", "密码错误");
            }
        }
        else
        {
            QMessageBox::warning(this, "错误", "该用户身份不是" + role);
        }
    }
    else
    {
        QMessageBox::warning(this, "错误", "该用户不存在");
    }
}
