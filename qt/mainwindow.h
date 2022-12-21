#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSqlQuery>
#include <QTableWidget>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonConnect_clicked();

    void on_comboBox_currentIndexChanged(int index);

    void on_pushButtonSearch_clicked();

    void on_pushButtonEditCV_clicked();

    void on_pushButtonOpenCV_clicked();

    void on_tableWidgetData_cellChanged(int row, int column);

    void on_pushButtonBack_clicked();

    void on_pushButtonSaveTable_clicked();

    void on_pushButtonDontSave_clicked();

    void on_tableWidgetData_cellPressed(int row, int column);

    void on_pushButtonAddVacancy_clicked();

    void on_pushButtonDeleteVacancy_clicked();

    void on_pushButtonRegistration_clicked();

    void on_pushButtonRegistrationSecond_clicked();

    void on_spinBoxAmountVacancies_valueChanged(int arg1);

    void on_comboBox_activated(int index);

    void on_pushButtonShowEmployees_clicked();

    void on_pushButtonAdminSave_clicked();

    void on_pushButtonAdmin_clicked();

    void on_pushButtonAdminDontSave_clicked();

    void on_pushButtonShowCompanies_clicked();

    void on_pushButtonAdminShowVacancy_clicked();

    void on_pushButtonDeleteVacancyAdmin_clicked();

private:
    Ui::MainWindow *ui;
    QSqlDatabase *db;
    int curCompany;//текущая компания открывающаяся администратором
    QString currentID;//id вошедшей компании или работника
    QString currentJobFormat;//id job_format вошедшей компании или работника
    QString cv_idValue;//id cv вошедшего работника
    bool EditStarted=false;//true если редактируется резюме пользователем или вакансии компанией
    bool CreatingStarted=false;//true если началось создание новых вакансий компанией
    bool LoginRepeated=false;//true если login введенный при регистрации уже занят
    bool CurrentTableOpened = false;//true если открыта таблица вакансий компанией или резюме сотрудником
    bool warning = false;//true если уже была показана информация о шаблоне ввода навыков
    bool registration = false;//true если началась регистрация компании или работника
    bool EmployeeAdminOpened = false;//true если администратор открыл данные сотрудников
    bool CompanyAdminOpened=false;//true еслиадминистратор открыл данные компаний
    int ok=0;//счетчик прошедших проверок открытой бд
    QString vacancy_id_opened[20];//массив id вакансий
    QString job_format_id_opened[20];//массив id job_format вакансий
    void LogIn(QString accountType, QString nick_column);//функция проверяющая логин и пароль работника или компании
    void ShowSearchedVacancies(QString valueToSearch);//функция показывающая вакансии по фразе поиска сотрудника
    void CreateCells(int rows, QTableWidget *currentTable);//создание ячеек в выбранно QTableWidget
    void FillTable(int rowsNumber, QSqlQuery currentQuery, QString skills, bool hide, int value0, int value1, int value3, int value4, int value5, int value6);//заполнение таблицы
    void EndSave();//окончание сохранения
    bool CheckSkills(QString currentSkills);//проверка правильности введенных навыков
public:

};

#endif // MAINWINDOW_H

