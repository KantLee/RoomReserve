// ----------------------------------------------------------
// kroomreserve.h
// 创建者： 李康
// 创建时间： 2024/4/20
// 功能描述： 会议室预约系统——系统人员登录首页
// ----------------------------------------------------------

#ifndef _KROOMRESERVE_KROOMRESERVE_H_
#define _KROOMRESERVE_KROOMRESERVE_H_

#include <QtWidgets/QWidget>
#include <QMessageBox>

#include "ui_kroomreserve.h"
#include "kdatabase.h"
#include "kadminwidget.h"
#include "kcoderwidget.h"
#include "kapprovalwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class KRoomReserveClass; };
QT_END_NAMESPACE

class KRoomReserve : public QWidget
{
    Q_OBJECT

public:
    friend class KRoomReserveUnitTest;
    explicit KRoomReserve(QWidget *parent = nullptr);
    ~KRoomReserve();

protected:
    void initDataBase();    // 初始化数据库

private slots:
    void on_loginButton_clicked();
    void on_exitButton_clicked();

private:
    KRoomReserve(const KRoomReserve &other) = delete;
    KRoomReserve &operator=(const KRoomReserve &other) = delete;

    Ui::KRoomReserveClass *ui;
    KDataBase &m_database;
    KAdminWidget *m_adminWindow;
    KApprovalWidget *m_approvalWindow;
    KCoderWidget *m_coderWindow;
};

#endif // _KROOMRESERVE_KROOMRESERVE_H_
