#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QChar>
#include <QCryptographicHash>

MainWindow::MainWindow(QWidget *parent) ://конструктор
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    db = nullptr;
    //скрываем и показываем необходимые элементы интерфейса
    ui->widgetAddVacancy->hide();
    ui->pushButtonEditCV->hide();
    ui->tableWidgetData->hide();
    ui->widgetSearchVacancy->hide();
    ui->widgetEditCV_Vacancy->hide();
    ui->pushButtonBack->hide();
    ui->widgetLogIn->move(400,ui->widgetLogIn->y());
    ui->widgetShowForAdmin->move(460,20);
    ui->widgetDeleteVacancy->hide();
    ui->tableWidgetRegistrCompany->hide();
    ui->tableWidgetRegistrEmployee->hide();
    ui->tableWidgetAdminCompanies->hide();
    ui->tableWidgetAdminVacancies->hide();
    ui->pushButtonRegistrationSecond->hide();
    ui->labelRegistration->hide();
    ui->pushButtonDontSave->hide();
    ui->widgetShowForAdmin->hide();
    ui->tableWidgetAdminEmployee->hide();
    ui->pushButtonAdmin->hide();
    ui->pushButtonAdminDontSave->hide();
    ui->pushButtonAdminSave->hide();
    ui->widgetAdminEditing->move(790,20);
    ui->tableWidgetAdminVacancies->move(310,510);
    ui->widgetAdminVacancies->hide();
    ui->widgetDeleteVacancyAdmin->hide();
}

MainWindow::~MainWindow()//деструктор
{
    delete ui;
}

void MainWindow::on_pushButtonConnect_clicked()//конектимся к базе данных
{
    if(ok!=2)//если все проверки были успешны заново не конектимся
    {
        if (db != nullptr)
        {
            db->close();
            delete db;
            db = nullptr;
            return;
        }
        else
            db = new QSqlDatabase(QSqlDatabase::addDatabase("QPSQL"));

        QString dbName{"job_db"};//создаем переменные для базы данных
        QString host{"127.0.0.1"};
        QString usr{"postgres"};
        QString pwd{"1234"};

        db->setDatabaseName(dbName);//задаем значения базе данных
        db->setHostName(host);
        db->setUserName(usr);
        db->setPassword(pwd);
        db->setPort(5432);

        if (db->isValid())
        {
            ok++;
            //QMessageBox::information(this, "Valid", "Yes");
        }
        else
            QMessageBox::information(this, "Valid", "No");//показываем информацию пользователю

        if (db->isDriverAvailable("QPSQL"))
        {
            ok++;
            //QMessageBox::information(this, "Driver available", "Yes");
        }
        else
            QMessageBox::information(this, "Driver available", "No");//показываем информацию пользователю

        if (!db->open())
        {
            QMessageBox::warning(this, "Error", db->lastError().text());
            delete db;
            db = nullptr;
        }
    }
    if(!registration)//если не регистрация аккаунта а вход
    {
        if(ui->comboBox->currentIndex()==0) // вход для компании
        {
            QString user = "company";
            QString columnName = "name";
            LogIn(user,columnName);
        }

        if(ui->comboBox->currentIndex()==1) // вход для работника
        {
            QString user = "employee";
            QString columnName = "initials";
            LogIn(user,columnName);
        }

        if(ui->comboBox->currentIndex()==2) // вход для администратора
        {
            if(ui->lineEditLogin->text()=="q" && ui->lineEditPassword->text()=="q") // проверяем логин пароль администратора
            {
                ui->widgetShowForAdmin->show();//скрываем и показываем необходимые элементы интерфейса
                ui->widgetLogIn->hide();
                ui->pushButtonBack->show();
                ui->lineEditLogin->clear();
                ui->lineEditPassword->clear();
            }
            else
                QMessageBox::information(this, "Информация", "Введены неправильные логин или пароль");// при неправильном логине или пароле выводим ошибку
        }
    }
}

void MainWindow::on_comboBox_currentIndexChanged(int)//при изменении типа аккаунта для входа очищаем поля логина и пароля
{
    ui->lineEditLogin->clear();
    ui->lineEditPassword->clear();
}

void MainWindow::LogIn (QString accountType, QString nick_column)//проверка пароля и логина и получение значений из базы данных
{
    ui->lineEditSearch->clear();
    QCryptographicHash hasher(QCryptographicHash::Md5);
    hasher.addData(ui->lineEditPassword->text().toUtf8());
    QString password = QString::fromUtf8(hasher.result().toHex());
    QString login=ui->lineEditLogin->text();

    QString columnPK;
    QSqlQuery qry;
    //создаем запрос из аккаунтов с введенными логином и паролем
    qry.prepare("SELECT * FROM "+accountType+" WHERE password = '"+password+"' AND login = '"+login+"'");
    qry.exec();
    if(qry.size()<1)//если аккаунтов с введенными логином и паролем нет выдаем ошибку
    {
        QMessageBox welcome(QMessageBox::Information, " ", "Не удалось найти аккаунт с такими данными", QMessageBox::NoButton, this);
        welcome.setBaseSize(QSize(600, 120));
        welcome.exec();
    }
    else//иначе входим
    {
        ui->pushButtonDontSave->hide();//скрываем и показываем необходимые элементы интерфейса
        ui->pushButtonEditCV->hide();
        ui->pushButtonOpenCV->show();
        ui->pushButtonSaveTable->hide();
        ui->lineEditLogin->clear();
        ui->lineEditPassword->clear();
        ui->pushButtonBack->show();
        ui->widgetEditCV_Vacancy->show();
        ui->widgetLogIn->hide();

        columnPK+=accountType+"_id";
        //получаем id работника или компании
        qry.prepare("SELECT "+columnPK+" FROM "+accountType+" WHERE password = '"+password+"' AND login = '"+login+"'");
        qry.exec();
        qry.first();
        currentID=qry.value(0).toString();

        //приветствие по названию компании или имени работника
        qry.prepare("SELECT "+nick_column+" FROM "+accountType+" WHERE password = '"+password+"' AND login = '"+login+"'");
        qry.exec();
        qry.first();
        ui->label_welcome->setText("Здравствуйте, " + qry.value(0).toString());
        ui->label_welcome->show();

        if(accountType=="employee")//если вошел сотрудник
        {
            qry.prepare("SELECT cv FROM "+accountType+" WHERE password = '"+password+"' AND login = '"+login+"'");//получаем cv_id
            qry.exec();
            qry.first();
            cv_idValue=qry.value(0).toString();

            qry.prepare("SELECT job_format FROM cv WHERE cv_id="+cv_idValue+"");//получаем job_format
            qry.exec();
            qry.first();
            currentJobFormat=qry.value(0).toString();
            ui->widgetSearchVacancy->show();
            ui->labelSearch_2->setText("Резюме");
        }
        else//если вошла компания
        {
            ui->pushButtonDeleteVacancy->setText("Удалить вакансию №");
            ui->spinBoxDeleteVacancy->setMaximum(20);
            ui->labelSearch_2->setText("Вакансии");
            //ui->widgetAddVacancy->show();
        }
    }
}

void MainWindow::ShowSearchedVacancies(QString valueToSearch)//показывает вакансии минимум с одним совпадающим словом с введенным работником в поиске
{
    QSqlQuery qwProfession,querySkills,qwAll;
    QString currentVacancyID,skills;
    int skillsAmount=0,rowsCount=0,rows=0,startIndex;
    valueToSearch=valueToSearch.toLower();
    bool add=false;
    qwProfession=db->exec("SELECT profession FROM vacancy ORDER BY vacancy_id");
    while (qwProfession.next())
    {
        QString currentProfession = qwProfession.value(0).toString().toLower();//профессия из бд
        startIndex=currentProfession.indexOf(valueToSearch);

        if(startIndex!=-1)//если профессия из бд содержит введенные работником в поиске слова
        {
            if (startIndex == 0 && valueToSearch.length()==currentProfession.length())
                rows++;

            else if(startIndex>0 && startIndex+valueToSearch.length()<currentProfession.length() && currentProfession[startIndex-1]==" "&& currentProfession[startIndex+valueToSearch.length()]==" ")
                rows++;

            else if ((startIndex==0) && startIndex+valueToSearch.length()<currentProfession.length() && currentProfession[startIndex+valueToSearch.length()]==" ")
                rows++;

            else if((startIndex>0)&&startIndex+valueToSearch.length()==currentProfession.length() && currentProfession[startIndex-1]==" ")
                rows++;
        }
    }
    if(rows==0)//если нет подходящих вакансий
        QMessageBox::information(this, "Информация", "К сожалению таких вакансий не найдено");//показываем информацию пользователю

    else//если подходящие вакансии есть выводим их
    {
        ui->pushButtonDontSave->hide();
        ui->pushButtonEditCV->hide();
        CurrentTableOpened=false;
        qwAll=db->exec("SELECT * FROM vacancy INNER JOIN job_format ON job_format_id=job_format ORDER BY vacancy_id");
        qwProfession=db->exec("SELECT profession FROM vacancy ORDER BY vacancy_id");
        ui->tableWidgetData->setRowCount(rows);//создаем столько строк в таблице сколько найденных профессий
        CreateCells(rows,ui->tableWidgetData);//создаем ячейки в таблице
        while(rows>0)//выводим профессии
        {
            while (qwAll.next())
            {
                qwProfession.next();
                QString currentProfession = qwProfession.value(0).toString().toLower();
                startIndex=currentProfession.indexOf(valueToSearch);
                add=false;
                if(startIndex!= -1)//если профессия из бд содержит введенные работником в поиске слова
                {
                    if (startIndex==0 && valueToSearch.length()==currentProfession.length())
                        add=true;//добавляем вакансию в вывод

                    else if(startIndex>0 && startIndex+valueToSearch.length()<currentProfession.length() && currentProfession[startIndex-1]==" "&& currentProfession[startIndex+valueToSearch.length()]==" ")
                        add=true;//добавляем вакансию в вывод

                    else if ((startIndex==0) && startIndex+valueToSearch.length()<currentProfession.length() && currentProfession[startIndex+valueToSearch.length()]==" ")
                        add=true;//добавляем вакансию в вывод

                    else if((startIndex>0)&&startIndex+valueToSearch.length()==currentProfession.length() && currentProfession[startIndex-1]==" ")
                        add=true;//добавляем вакансию в вывод

                    if(add)//выводим вакансию
                    {
                        currentVacancyID = qwAll.value(0).toString();
                        querySkills=db->exec("SELECT skill FROM skills_for_vacancy WHERE vacancy_id = "+currentVacancyID+" ORDER BY line_skills_for_vacancy_id");
                        while(querySkills.next())
                        {
                            if(skillsAmount==0)
                            {
                                skills+=querySkills.value(0).toString();
                                skillsAmount++;
                            }
                            else
                                skills+=", "+querySkills.value(0).toString();//заполняем переменную навыками
                        }
                        FillTable(rowsCount,qwAll,skills,false,3,6,9,8,1,5);//заполняем таблицу

                        skillsAmount=0;//обнуляем переменные
                        skills="";
                        rows--;//изменяем переменные счетчики
                        rowsCount++;
                    }
                }
            }
        }
        ui->tableWidgetData->resizeColumnsToContents();//изменяем размер столбцов таблицы под текст внутри
        ui->tableWidgetData->setEditTriggers(QAbstractItemView::NoEditTriggers); //запрет редактирования
        ui->tableWidgetData->show();
    }
}

void MainWindow::on_pushButtonSearch_clicked()//при нажатии на поиск вакансии сотрудником
{
    ShowSearchedVacancies(ui->lineEditSearch->text());
}

void MainWindow::on_pushButtonEditCV_clicked()//при нажатии на изменение резюме сотрудником
{
    if(CurrentTableOpened)//если открыто резюме сотрудника или вакансии компании разрешаем
    {
        ui->pushButtonDontSave->show();//скрываем и показываем необходимые элементы интерфейса
        ui->widgetDeleteVacancy->hide();
        ui->tableWidgetData->selectionModel()->clear();
        QObject::connect(ui->tableWidgetData, &QTableWidget::clicked,//конектим слот с таблицей
                         ui->tableWidgetData, QOverload<const QModelIndex&>::of(&QTableWidget::edit));
        EditStarted=true;
        ui->pushButtonSaveTable->show();//скрываем и показываем необходимые элементы интерфейса
        ui->pushButtonDontSave->show();
        ui->widgetSearchVacancy->hide();
        ui->pushButtonOpenCV->hide();
        ui->pushButtonEditCV->hide();
        if(ui->labelSearch_2->text()=="Вакансии")
            ui->widgetAddVacancy->hide();
    }
    else//если резюме сотрудника или вакансии компании не открыты не разрешаем редактировать
        if(ui->labelSearch_2->text()=="Резюме")
            QMessageBox::warning(this, "Предупреждение", "Редактировать можно только свое резюме");
        else
            QMessageBox::warning(this, "Предупреждение", "Редактировать можно только свои вакансии");
}

void MainWindow::on_pushButtonOpenCV_clicked()//показываем данные резюме сотруднику либо вакансий компании, в зависимости от того кто зашел
{
    ui->tableWidgetData->show();
    ui->pushButtonEditCV->show();

    ui->tableWidgetData->setRowCount(1);
    ui->pushButtonEditCV->show();
    QSqlQuery query, querySkills,qwAll;
    QString skills="",currentVacancyID;
    int skillsAmount=0,rowsCount=0,rows=0;
    if(ui->labelSearch_2->text()=="Резюме")//если вошел сотрудник открываем его резюме
    {
        CurrentTableOpened=true;
        ui->tableWidgetData->setRowCount(1);
        CreateCells(1,ui->tableWidgetData);
        qwAll=db->exec("SELECT * FROM cv INNER JOIN job_format ON job_format_id=job_format WHERE job_format="+currentJobFormat+"");
        while (qwAll.next())
        {
            querySkills=db->exec("SELECT skill FROM employee_skills WHERE cv_id_skill = "+cv_idValue+" ORDER BY line_employee_skills_id ");
            while(querySkills.next())
            {
                if(skillsAmount==0)//если навык первый то добавляем его без запятой перед ним
                {
                    skills+=querySkills.value(0).toString();
                    skillsAmount++;
                }
                else
                    skills+=", "+querySkills.value(0).toString();//заполняем переменную навыками
            }
            FillTable(rowsCount,qwAll,skills,true,2,4,7,6,1,0);//заполняем таблицу
        }
    }
    else//если вошла компания открываем ее вакансии
    {
        ui->widgetAddVacancy->show();
        ui->widgetDeleteVacancy->show();
        CurrentTableOpened=true;
        query=db->exec("SELECT company_id FROM vacancy");
        while (query.next())
        {
            if(query.value(0).toString()==currentID)
                rows++;//получаем количество строк в таблице вакансий
        }

        ui->tableWidgetData->setRowCount(rows);
        CreateCells(rows,ui->tableWidgetData);
        qwAll=db->exec("SELECT * FROM vacancy INNER JOIN job_format ON job_format_id=job_format WHERE company_id="+currentID+" ORDER BY vacancy_id");
        while (qwAll.next())
        {
            currentVacancyID = qwAll.value(0).toString();
            querySkills=db->exec("SELECT skill FROM skills_for_vacancy WHERE vacancy_id = "+currentVacancyID+" ORDER BY line_skills_for_vacancy_id");
            while(querySkills.next())
            {
                if(skillsAmount==0)//если навык первый то добавляем его без запятой перед ним
                {
                    skills+=querySkills.value(0).toString();
                    skillsAmount++;
                }
                else
                    skills+=", "+querySkills.value(0).toString();//заполняем переменную навыками
            }
            FillTable(rowsCount,qwAll,skills,false,3,6,9,8,1,5);//заполняем таблицу
            skills="";
            skillsAmount=0;
            rowsCount++;
        }
        qwAll=db->exec("SELECT vacancy_id FROM vacancy WHERE company_id="+currentID+" ORDER BY vacancy_id");
        int i = 0;
        while (qwAll.next())
        {
            vacancy_id_opened[i] = qwAll.value(0).toString();//заполняем массив вакансий компании
            i++;
        }
        int j = 0;
        for(int k = i; k>0;k--)
        {
            qwAll=db->exec("SELECT job_format FROM vacancy WHERE vacancy_id = "+vacancy_id_opened[j]+" ORDER BY vacancy_id");
            while (qwAll.next())
            {
                job_format_id_opened[j] = qwAll.value(0).toString();//заполняем массив формата работы компании
                j++;
            }
        }
    }
    ui->tableWidgetData->resizeColumnsToContents();//изменяем размер столбцов таблицы под текст внутри
    ui->tableWidgetData->setEditTriggers(QAbstractItemView::NoEditTriggers); //запрет редактирования
}

void MainWindow::CreateCells(int rows, QTableWidget *currentTable)//создаем ячейки в полученной QTableWidget
{
    int rowsCount=0;//текущее количество созданных строк
    while(rowsCount!=rows)//если текущее количество созданных строк не равно нужному создаем дальше
    {
        for(int i = 0; i<currentTable->columnCount(); i++)
        {
            if (currentTable->item(rowsCount, i) == nullptr)
            {
                QTableWidgetItem *ti = new QTableWidgetItem;
                currentTable->setItem(rowsCount, i, ti);//присваиваем item
            }
        }
        rowsCount++;//увеличиваем количество созданных строк
    }
}

void MainWindow::on_pushButtonBack_clicked()//при нажатии кнопки "назад" возвращаем пользователя в меню входа
{
    CurrentTableOpened=false;//возвращаем значения булевым переменным и скрываем и показываем необходимые элементы интерфейса
    EditStarted=false;
    CreatingStarted=false;
    warning=false;
    EmployeeAdminOpened=false;
    CompanyAdminOpened=false;
    ui->widgetShowForAdmin->hide();
    ui->widgetAddVacancy->hide();
    ui->widgetEditCV_Vacancy->hide();
    ui->widgetSearchVacancy->hide();
    ui->widgetLogIn->show();
    ui->widgetAdminVacancies->hide();
    ui->widgetDeleteVacancyAdmin->hide();
    ui->label_welcome->hide();
    ui->labelRegistration->hide();
    ui->widgetDeleteVacancy->hide();
    ui->pushButtonBack->hide();
    ui->pushButtonAdminDontSave->hide();
    ui->pushButtonAdminSave->hide();
    ui->pushButtonAdmin->hide();
    ui->tableWidgetAdminEmployee->hide();
    ui->tableWidgetAdminEmployee->setRowCount(0);
    ui->tableWidgetData->hide();
    ui->tableWidgetData->setRowCount(0);
    ui->tableWidgetRegistrCompany->hide();
    ui->tableWidgetRegistrCompany->setRowCount(0);
    ui->tableWidgetRegistrEmployee->hide();
    ui->tableWidgetRegistrEmployee->setRowCount(0);
    ui->tableWidgetAdminCompanies->hide();
    ui->tableWidgetAdminVacancies->hide();
    QObject::disconnect(ui->tableWidgetRegistrCompany, &QTableWidget::clicked,//запрещаем редактирование таблицы
                        ui->tableWidgetRegistrCompany, QOverload<const QModelIndex&>::of(&QTableWidget::edit));
    QObject::disconnect(ui->tableWidgetRegistrEmployee, &QTableWidget::clicked,//запрещаем редактирование таблицы
                        ui->tableWidgetRegistrEmployee, QOverload<const QModelIndex&>::of(&QTableWidget::edit));
    QObject::disconnect(ui->tableWidgetData, &QTableWidget::clicked,//запрещаем редактирование таблицы
                        ui->tableWidgetData, QOverload<const QModelIndex&>::of(&QTableWidget::edit));
}

void MainWindow::on_pushButtonSaveTable_clicked()//сохранеие изменений резюме сотрудника либо вакансий компании или сохранение созданных вакансий компании
{
    if(EditStarted)//сохраняем отредактированные резюме либо вакансии
    {
        if(ui->labelSearch_2->text()=="Резюме")//сохраняем данные резюме
        {
            ui->tableWidgetData->item(0,6)->setText("");
            int skillsAmount=0, commasInSkills=0;
            QString skills, tableSkills;
            bool salaryToDouble, checkSkillsComma;
            int cells=6, curMasIndex=0;
            double salary=ui->tableWidgetData->item(0,5)->text().toDouble(&salaryToDouble);//правильно ли заполнена ячейка зарплаты

            for(int i=0; i<ui->tableWidgetData->columnCount()-1;i++)//считаем все ли ячейки заполнены
                if(ui->tableWidgetData->item(0,i)->text()=="")
                    cells--;
                else
                    cells++;
            tableSkills=ui->tableWidgetData->item(0,2)->text();
            //проверка правильно ли введены навыкои
            checkSkillsComma=CheckSkills(tableSkills);//проверка правильно ли введены навыки

            if(salaryToDouble&&salary>=0&&cells==12&&!checkSkillsComma)//если ячейки заполены корректно обновляем резюме
            {
                QSqlQuery query;
                //обновляем резюме
                query.prepare("UPDATE cv SET salary = :salaryValue, profession = :professionValue, experience = :experienceValue WHERE cv_id =:cv_idValue;");
                query.bindValue(":salaryValue", ui->tableWidgetData->item(0,5)->text());
                query.bindValue(":professionValue", ui->tableWidgetData->item(0,0)->text());
                query.bindValue(":experienceValue", ui->tableWidgetData->item(0,1)->text());
                query.bindValue(":cv_idValue", cv_idValue);
                query.exec();
                //обновляем формат работы
                query.prepare("UPDATE job_format SET location = :locationValue, work_schedule = :work_scheduleValue WHERE job_format_id =:job_format_idValue;");
                query.bindValue(":locationValue", ui->tableWidgetData->item(0,4)->text());
                query.bindValue(":work_scheduleValue", ui->tableWidgetData->item(0,3)->text());
                query.bindValue(":job_format_idValue", currentJobFormat);
                query.exec();
                query.prepare("SELECT skill FROM employee_skills WHERE cv_id_skill="+cv_idValue+"");
                query.exec();
                while(query.next())//считаем сколько навыков текующего работника было в базе данных
                {
                    if(skillsAmount==0)//если навык первый то добавляем его без запятой перед ним
                    {
                        skills+=query.value(0).toString();
                        skillsAmount++;
                    }
                    else
                    {
                        skills+=", "+query.value(0).toString();
                        skillsAmount++;
                    }
                }
                for(int i = 0; i<tableSkills.length(); i++)//считаем сколько навыков работник ввел
                    if(tableSkills[i]==",")
                        commasInSkills++;

                //QString tableSkillsMas[commasInSkills+1];//создаем массив размерностью равному количеству навыков введенных работником
                QVector<QString> tableSkillsMas(commasInSkills+1);

                for(int i=0; i<tableSkills.length(); i++)//заполняем массив введенных навыков
                {
                    if(tableSkills[i]!=",")
                    {
                        if (i>0)
                        {
                            if(tableSkills[i-1]!=","&&tableSkills[i]!=" ")
                                tableSkillsMas[curMasIndex]+=tableSkills[i];

                            else if(tableSkills[i-1]==","&&tableSkills[i]!=" ")
                                tableSkillsMas[curMasIndex]+=tableSkills[i];

                            else if(i<tableSkills.length()-1)
                                if(tableSkills[i-1]!=","&&tableSkills[i-1]!=" "&&tableSkills[i]==" "&&tableSkills[i+1]!=" "&&tableSkills[i+1]!=",")
                                    tableSkillsMas[curMasIndex]+=tableSkills[i];
                        }
                        else
                            tableSkillsMas[curMasIndex]+=tableSkills[i];
                    }
                    else
                        curMasIndex++;
                }

                query.prepare("SELECT line_employee_skills_id FROM employee_skills WHERE cv_id_skill = "+cv_idValue+" ORDER BY line_employee_skills_id");
                query.exec();
                QVector<QString> lineSkillMas(query.size());

                //QString lineSkillMas[query.size()];//создаем массив размерностью равному количеству id строк навыков
                int k=0;
                while (query.next())//заполняем массив значениями id строк навыков
                {
                    lineSkillMas[k]=query.value(0).toString();
                    k++;
                }

                if(skills!=tableSkills)//если содержимое навыков было изменено
                {
                    if(commasInSkills+1==skillsAmount)//если количество навыков не изменилось
                    {
                        for(int i = 0; i<skillsAmount; i++)//обновляем навыки в базе данных
                        {
                            query.prepare("UPDATE employee_skills SET skill = :skillValue WHERE line_employee_skills_id =:line_employee_skills_idValue;");
                            query.bindValue(":skillValue", tableSkillsMas[i]);
                            query.bindValue(":line_employee_skills_idValue", lineSkillMas[i]);
                            query.exec();
                        }
                    }
                    else if (commasInSkills+1<skillsAmount)//если количество навыков уменьшилось
                    {
                        int dif=skillsAmount-(commasInSkills+1);//получаем количество удаленных навыков
                        for(int i = 0; i<dif; i++)//удаляем навыки из базы данных
                        {
                            query.prepare("DELETE FROM employee_skills WHERE line_employee_skills_id =:line_employee_skills_idValue;");
                            query.bindValue(":line_employee_skills_idValue", lineSkillMas[skillsAmount-1-i]);
                            query.exec();
                        }
                        for(int i = 0; i<(commasInSkills+1); i++)//обновляем навыки в базе данных
                        {
                            query.prepare("UPDATE employee_skills SET skill = :skillValue WHERE line_employee_skills_id =:line_employee_skills_idValue;");
                            query.bindValue(":skillValue", tableSkillsMas[i]);
                            query.bindValue(":line_employee_skills_idValue", lineSkillMas[i]);
                            query.exec();
                        }
                    }
                    else if (commasInSkills+1>skillsAmount)//если количество навыков увеличилось
                    {
                        query.prepare("SELECT line_employee_skills_id FROM employee_skills ORDER BY line_employee_skills_id");
                        query.exec();
                        query.last();
                        int lastSkillsID=query.value(0).toInt();
                        for(int i = skillsAmount; i<commasInSkills+1; i++)//добавляем навыки в базу данных
                        {
                            query.prepare("INSERT INTO employee_skills (line_employee_skills_id, cv_id_skill, skill) "
                                          "VALUES (:line_employee_skills_idValue, :cv_id_skillValue, :skill)");
                            query.bindValue(":line_employee_skills_idValue", lastSkillsID + 1);
                            query.bindValue(":cv_id_skillValue", cv_idValue);
                            query.bindValue(":skill", tableSkillsMas[i]);
                            query.exec();
                            lastSkillsID++;
                        }
                        for(int i = 0; i<skillsAmount; i++)//обновляем навыки в базе данных
                        {
                            query.prepare("UPDATE employee_skills SET skill = :skillValue WHERE line_employee_skills_id =:line_employee_skills_idValue;");
                            query.bindValue(":skillValue", tableSkillsMas[i]);
                            query.bindValue(":line_employee_skills_idValue", lineSkillMas[i]);
                            query.exec();
                        }
                    }
                }
                EndSave();
                ui->pushButtonDontSave->hide();
                ui->widgetSearchVacancy->show();
                lineSkillMas.clear();
                tableSkillsMas.clear();
            }
            else//при неправильно введенных данных выводим ошибки
            {
                if(cells<12)
                    QMessageBox::warning(this, "Предупреждение", "Ячейки должны быть заполнены");

                if(!(salaryToDouble&&salary>=0))
                    QMessageBox::warning(this, "Предупреждение", "Зарплата должна быть положительным числом или нулем");

                if(checkSkillsComma)
                    QMessageBox::warning(this, "Предупреждение", "Пример правильно введенной строки навыков: skill1, skill2, skill3");
            }

        }
        else if(ui->labelSearch_2->text()=="Вакансии")//сохраняем вакансии
        {
            int skillsAmount, commasInSkills;
            QString skills, tableSkills;
            bool salaryToDouble, checkSkillsComma, salaryAny=false,cellsAny=false,salaryToDoubleAny=false, checkSkillsCommaAny = false;
            int startCells=ui->tableWidgetData->rowCount()*7, curCells=startCells, curMasIndex=0, cv_index=-1;
            QSqlQuery query;

            query.prepare("SELECT vacancy_id FROM vacancy WHERE company_id = "+currentID+" ORDER BY vacancy_id");
            query.exec();
           // QString vacancy_id_line[query.size()];//создаем массив размерностью равному количеству id строк вакансий
            QVector<QString> vacancy_id_line(query.size());
            int k=0;
            while(query.next())
            {
                vacancy_id_line.insert(k, query.value(0).toString());
                k++;
            }

            query.prepare("SELECT job_format FROM vacancy WHERE company_id = "+currentID+" ORDER BY vacancy_id");
            query.exec();
            QString job_format_line(query.size());//создаем массив размерностью равному количеству id строк job_format
            k=0;
            while(query.next())
            {
                job_format_line.insert(k,query.value(0).toString());
                k++;
            }

            for(int i=0; i<k;i++)
            {
                double salary=ui->tableWidgetData->item(i,5)->text().toDouble(&salaryToDouble);

                if(salary<0)//проверка правильно ли введена зарплата
                    salaryAny=true;
                if(!salaryToDouble)
                    salaryToDoubleAny=true;

                for(int j=0; j<ui->tableWidgetData->columnCount();j++)//считаем все ли ячейки заполнены
                    if(ui->tableWidgetData->item(i,j)->text()=="")
                        curCells--;
                    else
                        curCells++;

                if((curCells!=startCells*2)&&(i==k-1))
                    cellsAny=true;

                tableSkills=ui->tableWidgetData->item(i,2)->text();

                checkSkillsComma=CheckSkills(tableSkills);//проверка правильно ли введены навыки
                if(checkSkillsComma)
                    checkSkillsCommaAny = true;
            }
            //при наличии неправильно введенных данных выдаем ошибки
            if(cellsAny)
                QMessageBox::warning(this, "Предупреждение", "Ячейки должны быть заполнены");

            else if(salaryToDoubleAny||salaryAny)
                QMessageBox::warning(this, "Предупреждение", "Зарплата должна быть положительным числом или нулем");

            else if(checkSkillsCommaAny)
                QMessageBox::warning(this, "Предупреждение", "Пример правильно введенной строки навыков: skill1, skill2, skill3");

            else for(int i=0; i<ui->tableWidgetData->rowCount();i++)//если все проверки правильны сохраняем вакансии
            {
                skills="";
                cv_index++;
                curMasIndex=0;
                tableSkills=ui->tableWidgetData->item(i,2)->text();
                skillsAmount=0;
                commasInSkills=0;
                query.prepare("SELECT line_skills_for_vacancy_id FROM skills_for_vacancy WHERE vacancy_id = "+vacancy_id_line[i]+" ORDER BY line_skills_for_vacancy_id");
                query.exec();
                int l=0;
                //QString line_skills_from_vacancy[query.size()];
                QVector<QString> line_skills_from_vacancy(query.size());
                while(query.next())
                {
                    line_skills_from_vacancy[l]=query.value(0).toString();
                    l++;
                }

                query.prepare("SELECT skill FROM skills_for_vacancy WHERE vacancy_id="+vacancy_id_line[i]+" ORDER BY line_skills_for_vacancy_id");
                query.exec();
                while(query.next())//считаем сколько навыков текующего работника было в базе данных
                {
                    if(skillsAmount==0)//если навык первый то добавляем его без запятой перед ним
                    {
                        skills+=query.value(0).toString();
                        skillsAmount++;
                    }
                    else
                    {
                        skills+=", "+query.value(0).toString();
                        skillsAmount++;
                    }
                }
                //обновляем вакансию
                query.prepare("UPDATE vacancy SET salary = :salaryValue, profession = :professionValue, contacts =:contactsValue, "
                              "experience = :experienceValue WHERE vacancy_id =:vacancy_idValue;");
                query.bindValue(":salaryValue", ui->tableWidgetData->item(i,5)->text());
                query.bindValue(":professionValue", ui->tableWidgetData->item(i,0)->text());
                query.bindValue(":contactsValue", ui->tableWidgetData->item(i,6)->text());
                query.bindValue(":experienceValue", ui->tableWidgetData->item(i,1)->text());
                query.bindValue(":vacancy_idValue", vacancy_id_line[i]);
                query.exec();
                //обновляем формат работы
                query.prepare("UPDATE job_format SET location = :locationValue, work_schedule = :work_scheduleValue WHERE job_format_id =:job_format_idValue;");
                query.bindValue(":locationValue", ui->tableWidgetData->item(i,4)->text());
                query.bindValue(":work_scheduleValue", ui->tableWidgetData->item(i,3)->text());
                query.bindValue(":job_format_idValue", job_format_line.at(i));
                query.exec();

                for(int i = 0; i<tableSkills.length(); i++)//считаем сколько навыков работадатель ввел
                    if(tableSkills[i]==",")
                        commasInSkills++;

                //QString tableSkillsMas[commasInSkills+1];//создаем массив размерностью равному количеству навыков введенных работадателем
                QVector<QString> tableSkillsMas(commasInSkills+1);

                for(int i=0; i<tableSkills.length(); i++)//заполняем массив введенных навыков
                {
                    if(tableSkills[i]!=",")
                    {
                        if (i>0)
                        {
                            if(tableSkills[i-1]!=","&&tableSkills[i]!=" ")
                            {
                                tableSkillsMas[curMasIndex]+=tableSkills[i];
                                if(i==tableSkills.length()-1)
                                    curMasIndex++;
                            }

                            else if(tableSkills[i-1]==","&&tableSkills[i]!=" ")
                            {
                                tableSkillsMas[curMasIndex]+=tableSkills[i];
                                if(i==tableSkills.length()-1)
                                    curMasIndex++;
                            }

                            else if(i<tableSkills.length()-1)
                                if(tableSkills[i-1]!=","&&tableSkills[i-1]!=" "&&tableSkills[i]==" "&&tableSkills[i+1]!=" "&&tableSkills[i+1]!=",")
                                    tableSkillsMas[curMasIndex]+=tableSkills[i];
                        }
                        else
                            tableSkillsMas[curMasIndex]+=tableSkills[i];
                    }
                    else
                        curMasIndex++;
                }

                if(skills!=tableSkills)//если содержимое навыков было изменено
                {
                    if(commasInSkills+1==skillsAmount)//если количество навыков не изменилось
                    {
                        for(int i = 0; i<skillsAmount; i++)//обновляем навыки в базе данных
                        {
                            query.prepare("UPDATE skills_for_vacancy SET skill = :skillValue WHERE line_skills_for_vacancy_id =:line_skills_for_vacancy_idValue;");
                            query.bindValue(":skillValue", tableSkillsMas[i]);
                            query.bindValue(":line_skills_for_vacancy_idValue", line_skills_from_vacancy[i]);
                            query.exec();
                        }
                    }

                    else if (commasInSkills+1<skillsAmount)//если количество навыков уменьшилось
                    {
                        int dif=skillsAmount-(commasInSkills+1);//получаем количество удаленных навыков
                        for(int i = 0; i<dif; i++)//удаляем навыки из базы данных
                        {
                            query.prepare("DELETE FROM skills_for_vacancy WHERE line_skills_for_vacancy_id =:line_skills_for_vacancy_idValue;");
                            query.bindValue(":line_skills_for_vacancy_idValue", line_skills_from_vacancy[skillsAmount-1-i]);
                            query.exec();
                        }
                        for(int i = 0; i<(commasInSkills+1); i++)//обновляем навыки в базе данных
                        {
                            query.prepare("UPDATE skills_for_vacancy SET skill = :skillValue WHERE line_skills_for_vacancy_id =:line_skills_for_vacancy_idValue;");
                            query.bindValue(":skillValue", tableSkillsMas[i]);
                            query.bindValue(":line_skills_for_vacancy_idValue", line_skills_from_vacancy[i]);
                            query.exec();
                        }
                    }

                    else if (commasInSkills+1>skillsAmount)//если количество навыков увеличилось
                    {
                        query.prepare("SELECT line_skills_for_vacancy_id FROM skills_for_vacancy ORDER BY line_skills_for_vacancy_id");
                        query.exec();
                        query.last();
                        int lastSkillsID=query.value(0).toInt();
                        for(int i = skillsAmount; i<commasInSkills+1; i++)//добавляем навыки в базу данных
                        {
                            query.prepare("INSERT INTO skills_for_vacancy (line_skills_for_vacancy_id, vacancy_id, skill) "
                                          "VALUES (:line_skills_for_vacancy_idValue, :vacancy_idValue, :skill)");
                            query.bindValue(":line_skills_for_vacancy_idValue", lastSkillsID + 1);
                            query.bindValue(":vacancy_idValue", vacancy_id_line[cv_index]);
                            query.bindValue(":skill", tableSkillsMas[i]);
                            query.exec();
                            lastSkillsID++;
                        }
                        for(int i = 0; i<skillsAmount; i++)//обновляем навыки в базе данных
                        {
                            query.prepare("UPDATE skills_for_vacancy SET skill = :skillValue WHERE line_skills_for_vacancy_id =:line_skills_for_vacancy_idValue;");
                            query.bindValue(":skillValue", tableSkillsMas[i]);
                            query.bindValue(":line_skills_for_vacancy_idValue", line_skills_from_vacancy[i]);
                            query.exec();
                        }
                    }
                }
                if(i==ui->tableWidgetData->rowCount()-1)//если сохранена последняя вакансия
                {
                    vacancy_id_line.clear();
                    line_skills_from_vacancy.clear();
                    tableSkillsMas.clear();
                    EndSave();
                    ui->pushButtonDontSave->hide();
                    ui->widgetAddVacancy->show();
                    ui->widgetDeleteVacancy->show();
                }
            }
        }
    }
    else//сохранение созданных вакансий
    {
        QString tableSkills;
        bool salaryToDouble, checkSkillsComma, salaryAny=false,checkSkillsCommaAny=false,cellsAny=false,salaryToDoubleAny=false;
        QSqlQuery query;
        int commasInSkills, curMasIndex, lastVacancy, lastJobFormat, lastLineSkill;

        query.prepare("SELECT vacancy_id FROM vacancy WHERE company_id="+currentID+"");
        query.exec();

        CreateCells(ui->tableWidgetData->rowCount(), ui->tableWidgetData);//создаем ячейки в таблице для новых вакансий

        if(ui->spinBoxAmountVacancies->value()+query.size()<21)
        {
            for(int i=0; i < ui->tableWidgetData->rowCount(); i++)//проверяем есть ли пустые ячейки в таблице
            {
                for(int j=0; j < ui->tableWidgetData->columnCount(); j++)
                {
                    cellsAny= ui->tableWidgetData->item(i,j)->text()==nullptr;
                    if(cellsAny)
                    {
                        break;
                    }
                }
            }

            if(!cellsAny)//если пустых ячеек нет проверяем введенные данные
            {
                for(int i=0; i<ui->tableWidgetData->rowCount();i++)
                {
                    double salary=ui->tableWidgetData->item(i,5)->text().toDouble(&salaryToDouble);

                    if(salary<0)//проверка правильно ли введена зарплата
                        salaryAny=true;
                    if(!salaryToDouble)
                        salaryToDoubleAny=true;

                    tableSkills=ui->tableWidgetData->item(i,2)->text();
                    checkSkillsComma=CheckSkills(tableSkills);//проверка правильно ли введены навыки

                    if(checkSkillsComma)
                        checkSkillsCommaAny=true;
                }
            }

            int rowCount = ui->tableWidgetData->rowCount();
            //при наличии неправильно введенных данных выдаем ошибки
            if(cellsAny)
                QMessageBox::warning(this, "Предупреждение", "Ячейки должны быть заполнены");

            else if(salaryToDoubleAny||salaryAny)
                QMessageBox::warning(this, "Предупреждение", "Зарплата должна быть положительным числом или нулем");

            else if(checkSkillsCommaAny)
                QMessageBox::warning(this, "Предупреждение", "Пример правильно введенной строки навыков: skill1, skill2, skill3");

            else for(int i=0; i<rowCount;i++)//данные таблицы проверены приступаем к сохранению в бд
            {
                commasInSkills=0;
                curMasIndex=0;
                tableSkills=ui->tableWidgetData->item(i,2)->text();

                for(int i = 0; i<tableSkills.length(); i++)//считаем сколько навыков работадатель ввел
                    if(tableSkills[i]==",")
                        commasInSkills++;

                //QString tableSkillsMas[commasInSkills+1];//создаем массив размерностью равному количеству навыков введенных работадателем
                 QVector<QString> tableSkillsMas(commasInSkills+1);

                for(int i=0; i<tableSkills.length(); i++)//заполняем массив введенных навыков
                {
                    if(tableSkills[i]!=",")
                    {
                        if (i>0)
                        {
                            if(tableSkills[i-1]!=","&&tableSkills[i]!=" ")
                                tableSkillsMas[curMasIndex]+=tableSkills[i];

                            else if(tableSkills[i-1]==","&&tableSkills[i]!=" ")
                                tableSkillsMas[curMasIndex]+=tableSkills[i];

                            else if(i<tableSkills.length()-1)
                                if(tableSkills[i-1]!=","&&tableSkills[i-1]!=" "&&tableSkills[i]==" "&&tableSkills[i+1]!=" "&&tableSkills[i+1]!=",")
                                    tableSkillsMas[curMasIndex]+=tableSkills[i];
                        }
                        else
                            tableSkillsMas[curMasIndex]+=tableSkills[i];
                    }
                    else
                        curMasIndex++;
                }

                query.prepare("SELECT vacancy_id FROM vacancy ORDER BY vacancy_id");
                query.exec();
                query.last();
                if(query.isValid())
                    lastVacancy = query.value(0).toInt();//если значение в бд есть берем последнее
                else
                    lastVacancy=0;//если значения в бд нет, присваиваем первый номер id

                query.prepare("SELECT job_format_id FROM job_format ORDER BY job_format_id");
                query.exec();
                query.last();
                if(query.isValid())
                    lastJobFormat = query.value(0).toInt();//если значение в бд есть берем последнее
                else
                    lastJobFormat=0;//если значения в бд нет, присваиваем первый номер id

                query.prepare("SELECT line_skills_for_vacancy_id FROM skills_for_vacancy ORDER BY line_skills_for_vacancy_id");
                query.exec();
                query.last();
                if(query.isValid())
                    lastLineSkill = query.value(0).toInt();//если значение в бд есть берем последнее
                else
                    lastLineSkill=0;//если значения в бд нет, присваиваем первый номер id

                //добавляем формат работы в базу данных
                query.prepare("INSERT INTO job_format (job_format_id, location, work_schedule) "
                              "VALUES (:job_format_idValue, :locationValue, :work_scheduleValue)");
                query.bindValue(":job_format_idValue", lastJobFormat + 1);
                query.bindValue(":locationValue", ui->tableWidgetData->item(i,4)->text());
                query.bindValue(":work_scheduleValue", ui->tableWidgetData->item(i,3)->text());
                query.exec();

                //добавляем вакансю в базу данных
                query.prepare("INSERT INTO vacancy (vacancy_id, salary, company_id, profession, job_format, contacts, experience) "
                              "VALUES (:vacancy_idValue, :salaryValue, :company_idValue, :professionValue, :job_formatValue, :contactsValue, :experienceValue)");
                query.bindValue(":vacancy_idValue", lastVacancy + 1);
                query.bindValue(":salaryValue", ui->tableWidgetData->item(i,5)->text());
                query.bindValue(":company_idValue", currentID);
                query.bindValue(":professionValue", ui->tableWidgetData->item(i,0)->text());
                query.bindValue(":job_formatValue", lastJobFormat + 1);
                query.bindValue(":contactsValue", ui->tableWidgetData->item(i,6)->text());
                query.bindValue(":experienceValue", ui->tableWidgetData->item(i,1)->text());
                query.exec();

                for(int i=0; i<commasInSkills+1;i++)
                {
                    //добавляем навык в базу данных
                    query.prepare("INSERT INTO skills_for_vacancy (line_skills_for_vacancy_id, vacancy_id, skill) "
                                  "VALUES (:line_skills_for_vacancy_idValue, :vacancy_idValue, :skill)");
                    query.bindValue(":line_skills_for_vacancy_idValue", lastLineSkill + 1);
                    query.bindValue(":vacancy_idValue", lastVacancy + 1);
                    query.bindValue(":skill", tableSkillsMas[i]);
                    query.exec();
                    lastLineSkill++;
                }
                lastVacancy++;
                lastJobFormat++;

                if(i==ui->tableWidgetData->rowCount()-1)
                {
                    tableSkillsMas.clear();
                    QMessageBox::information(this, "Добавление вакансии", "Вакансия добавлена.");//показываем информацию пользователю
                    on_pushButtonOpenCV_clicked();
                    ui->pushButtonBack->show();
                    ui->pushButtonEditCV->show();
                    ui->pushButtonDontSave->hide();
                    ui->pushButtonSaveTable->hide();
                    ui->pushButtonOpenCV->show();
                    ui->widgetAddVacancy->show();
                    CreatingStarted=false;
                    ui->pushButtonAddVacancy->setEnabled(true);
                    QObject::disconnect(ui->tableWidgetData, &QTableWidget::clicked,//запрещаем редактирование таблицы
                                        ui->tableWidgetData, QOverload<const QModelIndex&>::of(&QTableWidget::edit));
                }
            }
        }
        else
            QMessageBox::warning(this, "Предупреждение", "Вакансий не может быть больше 20");
    }
}

void MainWindow::on_pushButtonDontSave_clicked()//при нажатии на кнопку не сохранять возвращаем изначальные данные резюме либо вакансии
{
    QObject::disconnect(ui->tableWidgetData, &QTableWidget::clicked,//запрещаем редактирование таблицы
                        ui->tableWidgetData, QOverload<const QModelIndex&>::of(&QTableWidget::edit));
    EditStarted=false;
    CreatingStarted=false;
    on_pushButtonOpenCV_clicked();
    ui->pushButtonOpenCV->show();
    ui->pushButtonBack->show();

    if(ui->labelSearch_2->text()=="Резюме")
    {
        ui->widgetSearchVacancy->show();
    }
    if(ui->labelSearch_2->text()=="Вакансии")
    {
        ui->widgetDeleteVacancy->show();
        ui->widgetAddVacancy->show();
        ui->pushButtonAddVacancy->setEnabled(true);
    }
    warning=false;
}

void MainWindow::on_tableWidgetData_cellPressed(int row, int column)//если после начала редактирования ячейка с навыками была выбрана
{                                                                   //выводим информацию о шаблоне записи навыков
    if(row==0&&column==2&&EditStarted&&!warning)
    {
        QMessageBox::information(this, "Редактирование навыков", "Пример правильно введенной строки навыков: skill1, skill2, skill3");
        warning=true;
    }
}

void MainWindow::EndSave()//после окончания сейва выводим информацию об успешном сэйве и скрываем и показываем необходимые элементы интерфейса
{
    QMessageBox::information(this, "Отлично", "Данные успешно сохранены");//показываем информацию пользователю
    QObject::disconnect(ui->tableWidgetData, &QTableWidget::clicked,//запрещаем редактирование таблицы
                        ui->tableWidgetData, QOverload<const QModelIndex&>::of(&QTableWidget::edit));
    ui->tableWidgetData->resizeColumnsToContents();//изменяем размер столбцов таблицы под текст внутри
    EditStarted=false;
    ui->pushButtonSaveTable->hide();

    ui->pushButtonBack->show();
    ui->pushButtonOpenCV->show();
    ui->pushButtonEditCV->show();
    warning=false;
}

void MainWindow::FillTable(int rowsNumber, QSqlQuery currentQuery, QString skills, bool hide, int value0, int value1, int value3, int value4, int value5, int value6)
{   //заполняем таблицу
    ui->tableWidgetData->item(rowsNumber, 0)->setText(currentQuery.value(value0).toString());
    ui->tableWidgetData->item(rowsNumber, 1)->setText(currentQuery.value(value1).toString());
    ui->tableWidgetData->item(rowsNumber, 2)->setText(skills);
    ui->tableWidgetData->item(rowsNumber, 3)->setText(currentQuery.value(value3).toString());
    ui->tableWidgetData->item(rowsNumber, 4)->setText(currentQuery.value(value4).toString());
    ui->tableWidgetData->item(rowsNumber, 5)->setText(currentQuery.value(value5).toString());
    ui->tableWidgetData->setColumnHidden(6,hide);
    ui->tableWidgetData->item(rowsNumber, 6)->setText(currentQuery.value(value6).toString());
}

void MainWindow::on_tableWidgetData_cellChanged(int, int )
{   //Если после начала редактирования была изменена ячейка, то скрываем кнопку "назад" и ширину столбца подгоняем под ширину измененной ячейки
    if(EditStarted)
    {
        ui->pushButtonBack->hide();
        ui->widgetAddVacancy->hide();
        ui->tableWidgetData->resizeColumnsToContents();//изменяем размер столбцов таблицы под текст внутри
    }
    if(CreatingStarted)
    {
        ui->pushButtonBack->hide();
        ui->tableWidgetData->resizeColumnsToContents();//изменяем размер столбцов таблицы под текст внутри
    }
    ui->pushButtonSaveTable->show();
}

void MainWindow::on_pushButtonAddVacancy_clicked()//при нажатии на кнопку добавить вакансию
{
    if(ui->spinBoxAmountVacancies->value()+ui->tableWidgetData->rowCount()<21)//если в сумме не больше 20 вакансий то добавляем строку новой вакансии
    {
        EditStarted=false;
        CreatingStarted=true;
        ui->widgetDeleteVacancy->hide();
        ui->tableWidgetData->show();
        ui->pushButtonOpenCV->hide();
        ui->pushButtonEditCV->hide();
        ui->pushButtonSaveTable->show();
        ui->pushButtonDontSave->show();
        ui->tableWidgetData->setRowCount(0);
        ui->tableWidgetData->setRowCount(ui->spinBoxAmountVacancies->value());
        CreateCells(ui->spinBoxAmountVacancies->value(),ui->tableWidgetData);
        ui->tableWidgetData->resizeColumnsToContents();//изменяем размер столбцов таблицы под текст внутри
        QObject::connect(ui->tableWidgetData, &QTableWidget::clicked,//разрешаем редактирование
                         ui->tableWidgetData, QOverload<const QModelIndex&>::of(&QTableWidget::edit));
        ui->pushButtonAddVacancy->setEnabled(false);
    }
    else
        QMessageBox::warning(this, "Предупреждение", "Вакансий не может быть больше 20");
}

void MainWindow::on_pushButtonDeleteVacancy_clicked()//при нажатии на кнопку удалить
{
    QSqlQuery query;
    int lineNumber;
    lineNumber = ui->spinBoxDeleteVacancy->value();//получаем номер для удаления
    if(!EmployeeAdminOpened&&!CompanyAdminOpened)//удаление вакансий
    {
        if(ui->tableWidgetData->rowCount()==1 && lineNumber==1)
            QMessageBox::warning(this, "Предупреждение", "Нельзя удалить единственную вакансию.");
        else if(ui->tableWidgetData->rowCount()>=lineNumber)
        {
            //удаление навыков конкретной вакансии из базы данных
            query.prepare("DELETE FROM skills_for_vacancy WHERE vacancy_id =:vacancy_idValue;");
            query.bindValue(":vacancy_idValue", vacancy_id_opened[lineNumber-1]);
            query.exec();

            //удаление вакансии из базы данных
            query.prepare("DELETE FROM vacancy WHERE vacancy_id =:vacancy_idValue;");
            query.bindValue(":vacancy_idValue", vacancy_id_opened[lineNumber-1]);
            query.exec();

            //удаление формата работы из базы данных
            query.prepare("DELETE FROM job_format WHERE job_format_id =:job_format_idValue;");
            query.bindValue(":job_format_idValue", job_format_id_opened[lineNumber-1]);
            query.exec();

            QMessageBox::information(this, "Удаление вакансии", "Вакансия удалена.");//показываем информацию пользователю
            on_pushButtonOpenCV_clicked();
        }
        else if(ui->tableWidgetData->rowCount()<lineNumber)//если номера вакансии для удаления нет выдаем ошибку
            QMessageBox::warning(this, "Предупреждение", "Вакансии с таким номером нет.");
        ui->pushButtonBack->show();
    }
    else if (EmployeeAdminOpened)//удаление сотрудников администратором
    {
        if(lineNumber==0)
            QMessageBox::warning(this, "Предупреждение", "Все данные работников удалены.");
        else if(ui->tableWidgetAdminEmployee->rowCount()<lineNumber)//если номера сотрудника для удаления нет выдаем ошибку
            QMessageBox::warning(this, "Предупреждение", "Работника с таким номером нет.");
        else if(ui->tableWidgetAdminEmployee->rowCount()>=lineNumber)
        {
            QString cv,job_format,employee_id=ui->tableWidgetAdminEmployee->item(lineNumber-1,0)->text();
            query.prepare("SELECT cv FROM employee WHERE employee_id = "+employee_id+"");
            query.exec();
            query.first();
            cv=query.value(0).toString();

            query.prepare("SELECT job_format FROM cv WHERE cv_id = "+cv+"");
            query.exec();
            query.first();
            job_format=query.value(0).toString();

            //удаление навыков сотрудника из базы данных
            query.prepare("DELETE FROM employee_skills WHERE cv_id_skill =:cv_id_skillValue;");
            query.bindValue(":cv_id_skillValue", cv);
            query.exec();

            //удаление сотрудника из базы данных
            query.prepare("DELETE FROM employee WHERE employee_id =:employee_idValue;");
            query.bindValue(":employee_idValue", employee_id);
            query.exec();

            //удаление резюме сотрудника из базы данных
            query.prepare("DELETE FROM cv WHERE cv_id =:cv_idValue;");
            query.bindValue(":cv_idValue", cv);
            query.exec();

            //удаление формата работы сотрудника из базы данных
            query.prepare("DELETE FROM job_format WHERE job_format_id =:job_format_idValue;");
            query.bindValue(":job_format_idValue", job_format);
            query.exec();

            QMessageBox::information(this, "Удаление данных работника", "Данные успешно удалены.");//показываем информацию пользователю
            if(lineNumber==1 && ui->tableWidgetAdminEmployee->rowCount()==1)
            {
                ui->pushButtonAdmin->hide();
                ui->tableWidgetAdminEmployee->setRowCount(0);
                ui->tableWidgetAdminEmployee->hide();
                ui->widgetDeleteVacancy->hide();
                QMessageBox::information(this, "Информация", "Вы удалили данные всех работников.");//показываем информацию пользователю
            }
            else
                on_pushButtonShowEmployees_clicked();
        }
    }
    else if(CompanyAdminOpened)//удаление компании администратором
    {
        if(lineNumber==0)
            QMessageBox::warning(this, "Предупреждение", "Все данные компаний удалены");
        else if(ui->tableWidgetAdminCompanies->rowCount()<lineNumber)//если номера сотрудника для удаления нет выдаем ошибку
            QMessageBox::warning(this, "Предупреждение", "Компании с таким номером нет.");
        else if(ui->tableWidgetAdminCompanies->rowCount()>=lineNumber)
        {
            if (lineNumber==curCompany)
                ui->tableWidgetAdminVacancies->setRowCount(0);

            if(ui->tableWidgetAdminCompanies->rowCount()-1>0)
                ui->spinBoxVacanyNumber->setMaximum(ui->tableWidgetAdminCompanies->rowCount()-1);

            QString company_id=ui->tableWidgetAdminCompanies->item(lineNumber-1,0)->text();
            query.prepare("SELECT vacancy_id FROM vacancy WHERE company_id = "+company_id+"");
            query.exec();
             QVector<QString> vacancy_mas(query.size());
             QVector<QString> jobs_mas(query.size());
            int k=0;
            while(query.next())
            {
                vacancy_mas[k]=query.value(0).toString();
                k++;
            }

            query.prepare("SELECT job_format FROM vacancy WHERE company_id = "+company_id+"");
            query.exec();
            int j=0;
            while(query.next())
            {
                jobs_mas[j]=query.value(0).toString();
                j++;
            }

            for(int i=0; i<k; i++)
            {
                query.prepare("DELETE FROM skills_for_vacancy WHERE vacancy_id = "+vacancy_mas[i]+"");
                query.exec();
            }

            query.prepare("DELETE FROM vacancy WHERE company_id = "+company_id+"");
            query.exec();

            query.prepare("DELETE FROM company WHERE company_id = "+company_id+"");
            query.exec();

            for(int i=0; i<j; i++)
            {
                query.prepare("DELETE FROM job_format WHERE job_format_id="+jobs_mas[i]+"");
                query.exec();
            }
            QMessageBox::information(this, "Удаление данных компании", "Данные успешно удалены.");//показываем информацию пользователю
            if(lineNumber==1 && ui->tableWidgetAdminCompanies->rowCount()==1)
            {
                vacancy_mas.clear();
                jobs_mas.clear();
                ui->pushButtonAdmin->hide();
                ui->tableWidgetAdminCompanies->setRowCount(0);
                ui->tableWidgetAdminCompanies->hide();
                ui->tableWidgetAdminVacancies->hide();
                ui->widgetDeleteVacancy->hide();
                ui->widgetDeleteVacancyAdmin->hide();
                ui->widgetAdminVacancies->hide();
                QMessageBox::information(this, "Информация", "Вы удалили данные всех компаний.");//показываем информацию пользователю
            }
            else
            {
                vacancy_mas.clear();
                jobs_mas.clear();
                on_pushButtonShowCompanies_clicked();
            }
        }
    }
}

void MainWindow::on_pushButtonRegistration_clicked()//показываем необходимые объекты интерфейса для ввода данных для регистрации
{
    ui->pushButtonBack->show();
    ui->widgetLogIn->hide();
    ui->labelRegistration->move(220,ui->labelRegistration->y());
    ui->pushButtonRegistrationSecond->show();
    ui->labelRegistration->show();

    if(ui->comboBox->currentIndex()==0) // регистрация компании
    {
        ui->tableWidgetRegistrCompany->show();
        ui->tableWidgetRegistrCompany->setRowCount(1);
        CreateCells(1,ui->tableWidgetRegistrCompany);
        ui->tableWidgetRegistrCompany->resizeColumnsToContents();//изменяем размер столбцов таблицы под текст внутри
        ui->labelRegistration->setText("Регистрация компании");
        ui->tableWidgetRegistrCompany->move(0,ui->tableWidgetRegistrCompany->y());
        QObject::connect(ui->tableWidgetRegistrCompany, &QTableWidget::clicked,//разрешаем редактирование
                         ui->tableWidgetRegistrCompany, QOverload<const QModelIndex&>::of(&QTableWidget::edit));
    }

    if(ui->comboBox->currentIndex()==1) // регистрация работника
    {
        ui->tableWidgetRegistrEmployee->show();
        ui->tableWidgetRegistrEmployee->setRowCount(1);
        CreateCells(1,ui->tableWidgetRegistrEmployee);
        ui->tableWidgetRegistrEmployee->resizeColumnsToContents();//изменяем размер столбцов таблицы под текст внутри
        ui->labelRegistration->setText("Регистрация работника");
        ui->tableWidgetRegistrEmployee->move(220,ui->tableWidgetRegistrEmployee->y());
        QObject::connect(ui->tableWidgetRegistrEmployee, &QTableWidget::clicked,//разрешаем редактирование
                         ui->tableWidgetRegistrEmployee, QOverload<const QModelIndex&>::of(&QTableWidget::edit));
    }
}

void MainWindow::on_pushButtonRegistrationSecond_clicked() // проверяем данные и регистрируем компанию или работника
{
    registration=true;
    QString tableSkills;
    bool salaryToDouble, checkSkillsComma=false, salaryAny=false,cellsAny=false,salaryToDoubleAny=false;
    QSqlQuery query;
    int lastID, commasInSkills=0, curMasIndex=0, lastSkillsID;
    if(ui->labelRegistration->text()=="Регистрация работника")
    {
        for(int i=0; i < ui->tableWidgetRegistrEmployee->columnCount(); i++)//проверяем есть ли пустые ячейки
        {
            cellsAny= ui->tableWidgetRegistrEmployee->item(0,i)->text()==nullptr;
            if(cellsAny)
                break;
        }

        double salary=ui->tableWidgetRegistrEmployee->item(0,6)->text().toDouble(&salaryToDouble);//проверяем правильно ли записана зарплата

        if(salary<0)//проверка правильно ли введена зарплата
            salaryAny=true;

        if(!salaryToDouble)
            salaryToDoubleAny=true;

        tableSkills=ui->tableWidgetRegistrEmployee->item(0,3)->text();
        checkSkillsComma=CheckSkills(tableSkills);//проверка правильно ли введены навыки

        //при наличии неправильно введенных данных выдаем ошибки
        if(cellsAny)
            QMessageBox::warning(this, "Предупреждение", "Ячейки должны быть заполнены");

        else if(salaryToDoubleAny||salaryAny)
            QMessageBox::warning(this, "Предупреждение", "Зарплата должна быть положительным числом или нулем");

        else if(checkSkillsComma)
            QMessageBox::warning(this, "Предупреждение", "Пример правильно введенной строки навыков: skill1, skill2, skill3");

        else
        {
            for(int i = 0; i<tableSkills.length(); i++)//считаем сколько навыков работник ввел
                if(tableSkills[i]==",")
                    commasInSkills++;

            //QString tableSkillsMas[commasInSkills+1];//создаем массив размерностью равному количеству навыков введенных работником
            QVector<QString> tableSkillsMas(commasInSkills+1);


            for(int i=0; i<tableSkills.length(); i++)//заполняем массив введенных навыков
            {
                if(tableSkills[i]!=",")
                {
                    if (i>0)
                    {
                        if(tableSkills[i-1]!=","&&tableSkills[i]!=" ")
                            tableSkillsMas[curMasIndex]+=tableSkills[i];

                        else if(tableSkills[i-1]==","&&tableSkills[i]!=" ")
                            tableSkillsMas[curMasIndex]+=tableSkills[i];

                        else if(i<tableSkills.length()-1)
                            if(tableSkills[i-1]!=","&&tableSkills[i-1]!=" "&&tableSkills[i]==" "&&tableSkills[i+1]!=" "&&tableSkills[i+1]!=",")
                                tableSkillsMas[curMasIndex]+=tableSkills[i];
                    }
                    else
                        tableSkillsMas[curMasIndex]+=tableSkills[i];
                }
                else
                    curMasIndex++;
            }

            on_pushButtonConnect_clicked();

            if(db!=nullptr)
            {
                query=db->exec("SELECT employee_id FROM employee ORDER BY employee_id");//получаем id работника
                query.last();
                if(query.isValid())
                    lastID = query.value(0).toInt()+1;
                else
                    lastID=1;

                currentID=QString::number(lastID);
            }

            query.prepare("SELECT login FROM employee");
            query.exec();
            query.last();
            LoginRepeated=false;
            if(query.isValid())//проверяем нет ли таких же логинов
            {
                query.prepare("SELECT login FROM employee");
                query.exec();
                while(query.next())
                {
                    if(query.value(0).toString()==ui->tableWidgetRegistrEmployee->item(0,7)->text())
                        LoginRepeated=true;
                }
            }

            if(!LoginRepeated)//если вписан уникальный логин
            {
                ui->label_welcome->setText("Здравствуйте, "+ ui->tableWidgetRegistrEmployee->item(0,0)->text());
                ui->label_welcome->show();

                query.prepare("SELECT line_employee_skills_id FROM employee_skills ORDER BY line_employee_skills_id");
                query.exec();
                query.last();
                if(query.isValid())
                    lastSkillsID=query.value(0).toInt();//если значение в бд есть берем последнее
                else
                    lastSkillsID = 0;//если значения в бд нет задаем первое

                query.prepare("SELECT cv_id FROM cv ORDER BY cv_id");
                query.exec();
                query.last();
                if(query.isValid())
                    lastID = query.value(0).toInt()+1;//если значение в бд есть берем последнее
                else
                    lastID = 1;//если значения в бд нет задаем первое
                cv_idValue=QString::number(lastID);

                query.prepare("SELECT job_format_id FROM job_format ORDER BY job_format_id");
                query.exec();
                query.last();
                if(query.isValid())
                    lastID = query.value(0).toInt() + 1;//если значение в бд есть берем последнее
                else
                    lastID = 1;//если значения в бд нет задаем первое
                currentJobFormat=QString::number(lastID);

                ui->labelSearch_2->setText("Резюме");

                //добавляем формат работы в базу данных
                query.prepare("INSERT INTO job_format (job_format_id, location, work_schedule) "
                              "VALUES (:job_format_idValue, :locationValue, :work_scheduleValue)");
                query.bindValue(":job_format_idValue", currentJobFormat);
                query.bindValue(":locationValue", ui->tableWidgetRegistrEmployee->item(0,5)->text());
                query.bindValue(":work_scheduleValue", ui->tableWidgetRegistrEmployee->item(0,4)->text());
                query.exec();

                //добавляем резюме в базу данных
                query.prepare("INSERT INTO cv (cv_id, salary, profession, job_format, experience) "//отсюда
                              "VALUES (:cv_idValue, :salaryValue, :professionValue, :job_formatValue, :experienceValue)");
                query.bindValue(":cv_idValue", cv_idValue);
                query.bindValue(":salaryValue", ui->tableWidgetRegistrEmployee->item(0,6)->text());
                query.bindValue(":professionValue", ui->tableWidgetRegistrEmployee->item(0,1)->text());
                query.bindValue(":job_formatValue", currentJobFormat);
                query.bindValue(":experienceValue", ui->tableWidgetRegistrEmployee->item(0,2)->text());
                query.exec();

                QCryptographicHash hasher(QCryptographicHash::Md5);
                hasher.addData(ui->tableWidgetRegistrEmployee->item(0,8)->text().toUtf8());
                QString password = QString::fromUtf8(hasher.result().toHex());//хэшируем пароль вписанный пользователем

                //добавляем работника в базу данных
                query.prepare("INSERT INTO employee (employee_id, initials, cv, login, password) "
                              "VALUES (:employee_idValue, :initialsValue, :cvValue, :loginValue, :passwordValue)");
                query.bindValue(":employee_idValue", currentID);
                query.bindValue(":initialsValue", ui->tableWidgetRegistrEmployee->item(0,0)->text());
                query.bindValue(":cvValue", cv_idValue);
                query.bindValue(":loginValue", ui->tableWidgetRegistrEmployee->item(0,7)->text());
                query.bindValue(":passwordValue", password);//хэшем сделать
                query.exec();

                for(int i = 0; i<commasInSkills+1; i++)//добавляем навыки в базу данных
                {
                    query.prepare("INSERT INTO employee_skills (line_employee_skills_id, cv_id_skill, skill) "
                                  "VALUES (:line_employee_skills_idValue, :cv_id_skillValue, :skill)");
                    query.bindValue(":line_employee_skills_idValue", lastSkillsID + 1);
                    query.bindValue(":cv_id_skillValue", cv_idValue);
                    query.bindValue(":skill", tableSkillsMas[i]);
                    query.exec();
                    lastSkillsID++;
                }
                ui->widgetSearchVacancy->show();//скрываем и показываем необходимые элементы интерфейса
                ui->pushButtonRegistrationSecond->hide();
                ui->tableWidgetRegistrEmployee->hide();
                ui->labelRegistration->hide();
                ui->pushButtonEditCV->show();
                ui->widgetEditCV_Vacancy->show();
                ui->pushButtonBack->show();
                ui->lineEditSearch->clear();
                registration=false;
                QMessageBox::information(this, "Информация", "Регистрация завершена.");//показываем информацию пользователю
                ui->tableWidgetRegistrEmployee->setRowCount(0);
                QObject::disconnect(ui->tableWidgetRegistrEmployee, &QTableWidget::clicked,//запрещаем редактирование таблицы
                                    ui->tableWidgetRegistrEmployee, QOverload<const QModelIndex&>::of(&QTableWidget::edit));
                tableSkillsMas.clear();
            }
            else
            {
                tableSkillsMas.clear();
                QMessageBox::warning(this, "Предупреждение", "Введенный логин занят.");
            }
        }

    }
    else//регистрация компании
    {
        for(int i=0; i < ui->tableWidgetRegistrCompany->columnCount(); i++)//проверяем есть ли пустые ячейки
        {
            cellsAny= ui->tableWidgetRegistrCompany->item(0,i)->text()==nullptr;
            if(cellsAny)
                break;
        }

        if(!cellsAny)//если пустых ячеек нет
        {
            on_pushButtonConnect_clicked();
            double salary=ui->tableWidgetRegistrCompany->item(0,8)->text().toDouble(&salaryToDouble);

            if(salary<0)//проверка правильно ли  введена зарплата
                salaryAny=true;

            if(!salaryToDouble)
                salaryToDoubleAny=true;

            tableSkills=ui->tableWidgetRegistrCompany->item(0,5)->text();
            checkSkillsComma=CheckSkills(tableSkills);//проверка правильно ли введены навыки

            QSqlQuery queryLogin (*db);
            queryLogin.prepare("SELECT login FROM company");
            queryLogin.exec();
            queryLogin.last();
            LoginRepeated=false;
            if(queryLogin.isValid())//проверяем нет ли таких же логинов
            {
                queryLogin.prepare("SELECT login FROM company");
                queryLogin.exec();
                while(queryLogin.next())
                {
                    if(queryLogin.value(0).toString()==ui->tableWidgetRegistrCompany->item(0,10)->text())
                        LoginRepeated=true;
                }
            }
        }
        //при наличии неправильно введенных данных выдаем ошибки
        if(cellsAny)
            QMessageBox::warning(this, "Предупреждение", "Ячейки должны быть заполнены");

        else if(salaryToDoubleAny||salaryAny)
            QMessageBox::warning(this, "Предупреждение", "Зарплата должна быть положительным числом или нулем");

        else if(checkSkillsComma)
            QMessageBox::warning(this, "Предупреждение", "Пример правильно введенной строки навыков: skill1, skill2, skill3");

        else if(LoginRepeated)
            QMessageBox::warning(this, "Предупреждение", "Введенный логин занят.");

        else
        {
            QString lastVacancyID, LastJobFormatID;
            for(int i = 0; i<tableSkills.length(); i++)//считаем сколько навыков админ компании ввел
                if(tableSkills[i]==",")
                    commasInSkills++;

            //QString tableSkillsMas[commasInSkills+1];//создаем массив размерностью равному количеству навыков введенных админом компании
            QVector<QString> tableSkillsMas(commasInSkills+1);


            for(int i=0; i<tableSkills.length(); i++)//заполняем массив введенных навыков
            {
                if(tableSkills[i]!=",")
                {
                    if (i>0)
                    {
                        if(tableSkills[i-1]!=","&&tableSkills[i]!=" ")
                            tableSkillsMas[curMasIndex]+=tableSkills[i];

                        else if(tableSkills[i-1]==","&&tableSkills[i]!=" ")
                            tableSkillsMas[curMasIndex]+=tableSkills[i];

                        else if(i<tableSkills.length()-1)
                            if(tableSkills[i-1]!=","&&tableSkills[i-1]!=" "&&tableSkills[i]==" "&&tableSkills[i+1]!=" "&&tableSkills[i+1]!=",")
                                tableSkillsMas[curMasIndex]+=tableSkills[i];
                    }
                    else
                        tableSkillsMas[curMasIndex]+=tableSkills[i];
                }
                else
                    curMasIndex++;
            }

            if(db!=nullptr)
            {
                query=db->exec("SELECT company_id FROM company ORDER BY company_id");//получаем id компании
                query.last();
                if(query.isValid())
                    lastID = query.value(0).toInt()+1;
                else
                    lastID=1;//если значения в бд нет задаем первое
                currentID=QString::number(lastID);
            }
            ui->label_welcome->setText("Здравствуйте, "+ ui->tableWidgetRegistrCompany->item(0,0)->text());
            ui->label_welcome->show();

            query.prepare("SELECT line_skills_for_vacancy_id FROM skills_for_vacancy ORDER BY line_skills_for_vacancy_id");
            query.exec();
            query.last();
            if(query.isValid())
                lastSkillsID=query.value(0).toInt();//если значение в бд есть берем последнее
            else
                lastSkillsID=0;//если значения в бд нет задаем первое

            query.prepare("SELECT vacancy_id FROM vacancy ORDER BY vacancy_id");
            query.exec();
            query.last();
            if(query.isValid())
                lastID = query.value(0).toInt()+1;//если значение в бд есть берем последнее
            else
                lastID=1;//если значения в бд нет задаем первое
            lastVacancyID=QString::number(lastID);

            query.prepare("SELECT job_format_id FROM job_format ORDER BY job_format_id");
            query.exec();
            query.last();
            if(query.isValid())
                lastID = query.value(0).toInt() + 1;//если значение в бд есть берем последнее
            else
                lastID=1;//если значения в бд нет задаем первое
            LastJobFormatID=QString::number(lastID);
            ui->labelSearch_2->setText("Вакансии");

            //добавляем формат работы в базу данных
            query.prepare("INSERT INTO job_format (job_format_id, location, work_schedule) "
                          "VALUES (:job_format_idValue, :locationValue, :work_scheduleValue)");
            query.bindValue(":job_format_idValue", LastJobFormatID);
            query.bindValue(":locationValue", ui->tableWidgetRegistrCompany->item(0,7)->text());
            query.bindValue(":work_scheduleValue", ui->tableWidgetRegistrCompany->item(0,6)->text());
            query.exec();

            QCryptographicHash hasher(QCryptographicHash::Md5);
            hasher.addData(ui->tableWidgetRegistrCompany->item(0,11)->text().toUtf8());
            QString password = QString::fromUtf8(hasher.result().toHex());//хэшируем пароль вписанный пользователем

            //добавляем компанию в базу данных
            query.prepare("INSERT INTO company (company_id, description, name, field_of_activity, login, password) "
                          "VALUES (:company_idValue, :descriptionValue, :nameValue, :field_of_activityValue, :loginValue, :passwordValue)");
            query.bindValue(":company_idValue", currentID);
            query.bindValue(":descriptionValue", ui->tableWidgetRegistrCompany->item(0,2)->text());
            query.bindValue(":nameValue", ui->tableWidgetRegistrCompany->item(0,0)->text());
            query.bindValue(":field_of_activityValue", ui->tableWidgetRegistrCompany->item(0,1)->text());
            query.bindValue(":loginValue", ui->tableWidgetRegistrCompany->item(0,10)->text());
            query.bindValue(":passwordValue", password);
            query.exec();

            //добавляем вакансию в базу данных
            query.prepare("INSERT INTO vacancy (vacancy_id, salary, company_id, profession, job_format, contacts, experience) "
                          "VALUES (:vacancy_idValue, :salaryValue, :company_idValue, :professionValue, :job_formatValue, :contactsValue, :experienceValue)");
            query.bindValue(":vacancy_idValue", lastVacancyID);
            query.bindValue(":salaryValue", ui->tableWidgetRegistrCompany->item(0,8)->text());
            query.bindValue(":company_idValue", currentID);
            query.bindValue(":professionValue", ui->tableWidgetRegistrCompany->item(0,3)->text());
            query.bindValue(":job_formatValue", LastJobFormatID);
            query.bindValue(":contactsValue", ui->tableWidgetRegistrCompany->item(0,9)->text());
            query.bindValue(":experienceValue", ui->tableWidgetRegistrCompany->item(0,4)->text());
            query.exec();

            //добавляем навыки в базу данных
            for(int i = 0; i<commasInSkills+1; i++)
            {
                query.prepare("INSERT INTO skills_for_vacancy (line_skills_for_vacancy_id, vacancy_id, skill) "
                              "VALUES (:line_skills_for_vacancy_idValue, :vacancy_idValue, :skill)");
                query.bindValue(":line_skills_for_vacancy_idValue", lastSkillsID + 1);
                query.bindValue(":vacancy_idValue", lastVacancyID);
                query.bindValue(":skill", tableSkillsMas[i]);
                query.exec();
                lastSkillsID++;
            }
            tableSkillsMas.clear();
            ui->pushButtonDeleteVacancy->setText("Удалить вакансию №");//скрываем и показываем необходимые элементы интерфейса
            ui->pushButtonRegistrationSecond->hide();
            ui->tableWidgetRegistrCompany->hide();
            ui->labelRegistration->hide();
            ui->widgetEditCV_Vacancy->show();
            ui->labelSearch_2->show();
            ui->lineEditSearch->clear();
            ui->pushButtonEditCV->show();
            ui->pushButtonBack->show();
            registration=false;
            QObject::disconnect(ui->tableWidgetRegistrCompany, &QTableWidget::clicked,//запрещаем редактирование таблицы
                                ui->tableWidgetRegistrCompany, QOverload<const QModelIndex&>::of(&QTableWidget::edit));
            QMessageBox::information(this, "Информация", "Регистрация завершена.");//показываем информацию пользователю
            ui->tableWidgetRegistrCompany->setRowCount(0);
        }
    }
}

void MainWindow::on_spinBoxAmountVacancies_valueChanged(int)//если тип аккаунта при входе изменен очищаем поля логина и пароля
{
    if(CreatingStarted)
        ui->tableWidgetData->setRowCount(ui->spinBoxAmountVacancies->value());
}

bool MainWindow::CheckSkills(QString currentSkills)//функция для проверки корректности ввода навыков вакансии и резюме
{
    bool rightSkills = false;
    rightSkills=currentSkills.contains(",,", Qt::CaseInsensitive);

    if(!rightSkills)
        rightSkills=currentSkills.contains(", ,", Qt::CaseInsensitive);

    if(!rightSkills)
        rightSkills=currentSkills.contains(" ,", Qt::CaseInsensitive);

    if(!rightSkills)
        rightSkills=currentSkills.contains("  ", Qt::CaseInsensitive);

    if(!rightSkills)
        rightSkills=currentSkills.endsWith(",");

    if(!rightSkills)
        rightSkills=currentSkills.endsWith(" ");

    if(!rightSkills)
        rightSkills=currentSkills.startsWith(",");

    if(!rightSkills)
        rightSkills=currentSkills.startsWith(" ");

    return rightSkills;
}


void MainWindow::on_comboBox_activated(int)//убираем кнопку зарегистрироваться если входит админ
{
    if(ui->comboBox->currentIndex()==2)
        ui->pushButtonRegistration->hide();
    else
        ui->pushButtonRegistration->show();
}


void MainWindow::on_pushButtonShowEmployees_clicked()//показываем данные всех сотрудников
{
    QSqlQuery qwAll, querySkills, queryCV;
    int skillsAmount = 0 ,currentRow = -1;
    qwAll.prepare("SELECT * FROM cv INNER JOIN job_format ON job_format_id=job_format INNER JOIN employee ON cv=cv_id ORDER BY cv_id");
    qwAll.exec();
    if(qwAll.size()>0)//если в бд есть данные о сотрудниках
    {
        QObject::disconnect(ui->tableWidgetAdminEmployee,nullptr,nullptr,nullptr);
        ui->tableWidgetAdminEmployee->setEditTriggers(QAbstractItemView::NoEditTriggers);//запрещаем редактирование таблицы

        ui->widgetAdminVacancies->hide();
        ui->widgetDeleteVacancyAdmin->hide();
        ui->pushButtonAdmin->show();
        QString skills;
        QVector<QString> skillsMas(qwAll.size());


        ui->tableWidgetAdminEmployee->setRowCount(qwAll.size());
        CreateCells(qwAll.size(), ui->tableWidgetAdminEmployee);

        EmployeeAdminOpened=true;
        CompanyAdminOpened=false;

        ui->spinBoxDeleteVacancy->setMaximum(ui->tableWidgetAdminEmployee->rowCount());
        ui->tableWidgetAdminEmployee->show();
        ui->pushButtonDeleteVacancy->setText("Удалить работника №");
        ui->widgetDeleteVacancy->show();

        queryCV.prepare("SELECT cv_id FROM cv ORDER BY cv_id");
        queryCV.exec();

        for(int row=0; row<ui->tableWidgetAdminEmployee->rowCount(); row++){
            for(int col=0; col< ui->tableWidgetAdminEmployee->columnCount(); col++){
                auto item=new QTableWidgetItem;
                if(col==0 || col==7 || col==8|| col==9)
                    item->setFlags(item->flags() &  ~Qt::ItemIsEditable); //<--- нередактируемые столбцы
                ui->tableWidgetAdminEmployee->setItem(row, col, item);
            }
        }

        while (qwAll.next())
        {
            queryCV.next();
            currentRow++;
            skills="";
            skillsAmount=0;

            querySkills.prepare("SELECT skill FROM employee_skills WHERE cv_id_skill = "+queryCV.value(0).toString()+" ORDER BY line_employee_skills_id ");
            querySkills.exec();
            while(querySkills.next())
            {
                if(skillsAmount==0)//если навык первый то добавляем его без запятой перед ним
                {
                    skills+=querySkills.value(0).toString();
                    skillsAmount++;
                }
                else
                    skills+=", "+querySkills.value(0).toString();//заполняем переменную навыками
            }
            ui->tableWidgetAdminEmployee->item(currentRow, 0)->setText(qwAll.value(8).toString());//заполняем таблицу
            ui->tableWidgetAdminEmployee->item(currentRow, 1)->setText(qwAll.value(9).toString());
            ui->tableWidgetAdminEmployee->item(currentRow, 2)->setText(qwAll.value(2).toString());
            ui->tableWidgetAdminEmployee->item(currentRow, 3)->setText(qwAll.value(4).toString());
            ui->tableWidgetAdminEmployee->item(currentRow, 4)->setText(skills);
            ui->tableWidgetAdminEmployee->item(currentRow, 5)->setText(qwAll.value(7).toString());
            ui->tableWidgetAdminEmployee->item(currentRow, 6)->setText(qwAll.value(6).toString());
            ui->tableWidgetAdminEmployee->item(currentRow, 7)->setText(qwAll.value(1).toString());
            ui->tableWidgetAdminEmployee->item(currentRow, 8)->setText(qwAll.value(11).toString());
            ui->tableWidgetAdminEmployee->item(currentRow, 9)->setText(qwAll.value(12).toString());
        }


        skillsMas.clear();
        ui->tableWidgetAdminEmployee->resizeColumnsToContents();//изменяем размер столбцов таблицы под текст внутри
        ui->tableWidgetAdminCompanies->hide();
        ui->tableWidgetAdminVacancies->hide();
    }
    else
        QMessageBox::information(this, "Информация", "Данных о работниках нет");//показываем информацию пользователю
}

void MainWindow::on_pushButtonAdminSave_clicked()//сохраняем таблицы редактированные админом
{
    int skillsAmount=0, commasInSkills=0, currentRow=0;
    QString skills, tableSkills;
    bool checkSkillsCommaAny=false, currentCell=false, cellsAny=false;
    int curMasIndex=0;

    if(EmployeeAdminOpened)
    {
        for(int i=0; i < ui->tableWidgetAdminEmployee->rowCount(); i++)//проверяем есть ли пустые ячейки в таблице
        {
            for(int j=0; j < ui->tableWidgetAdminEmployee->columnCount(); j++)
            {
                currentCell = ui->tableWidgetAdminEmployee->item(i,j)->text()==nullptr;
                if(currentCell)
                    cellsAny=true;
            }
        }
        for(int i=0; i < ui->tableWidgetAdminEmployee->rowCount(); i++)//проверяем правильно ли введены навыки
            if(CheckSkills(ui->tableWidgetAdminEmployee->item(i,4)->text()))
                checkSkillsCommaAny=true;

        if(cellsAny)
            QMessageBox::warning(this, "Предупреждение", "Ячейки должны быть заполнены");

        else if(checkSkillsCommaAny)
            QMessageBox::warning(this, "Предупреждение", "Пример правильно введенной строки навыков: skill1, skill2, skill3");

        else if(!cellsAny&&!checkSkillsCommaAny)
        {
            for(int i = 0; i < ui->tableWidgetAdminEmployee->rowCount(); i++)
            {
                tableSkills=ui->tableWidgetAdminEmployee->item(currentRow,4)->text();
                skillsAmount=0;
                skills="";
                curMasIndex=0;
                commasInSkills=0;

                QString cv,job_format,employee_id=ui->tableWidgetAdminEmployee->item(currentRow,0)->text();
                QSqlQuery query;
                query.prepare("SELECT cv FROM employee WHERE employee_id = "+employee_id+"");
                query.exec();
                query.first();
                cv=query.value(0).toString();

                query.prepare("SELECT job_format FROM cv WHERE cv_id = "+cv+"");
                query.exec();
                query.first();
                job_format=query.value(0).toString();

                query.prepare("UPDATE employee SET initials = :initialsValue WHERE employee_id = :employee_idValue;");
                query.bindValue(":initialsValue", ui->tableWidgetAdminEmployee->item(currentRow,1)->text());
                query.bindValue(":employee_idValue", employee_id);
                query.exec();

                query.prepare("UPDATE cv SET profession = :professionValue, experience = :experienceValue WHERE cv_id =:cv_idValue;");
                query.bindValue(":professionValue", ui->tableWidgetAdminEmployee->item(currentRow,2)->text());
                query.bindValue(":experienceValue", ui->tableWidgetAdminEmployee->item(currentRow,3)->text());
                query.bindValue(":cv_idValue", cv);
                query.exec();

                query.prepare("UPDATE job_format SET location = :locationValue, work_schedule = :work_scheduleValue WHERE job_format_id =:job_format_idValue;");
                query.bindValue(":locationValue", ui->tableWidgetAdminEmployee->item(currentRow,6)->text());
                query.bindValue(":work_scheduleValue", ui->tableWidgetAdminEmployee->item(currentRow,5)->text());
                query.bindValue(":job_format_idValue", job_format);
                query.exec();

                query.prepare("SELECT skill FROM employee_skills WHERE cv_id_skill="+cv+"");
                query.exec();
                while(query.next())//считаем сколько навыков текующего работника было в базе данных
                {
                    if(skillsAmount==0)//если навык первый то добавляем его без запятой перед ним
                    {
                        skills+=query.value(0).toString();
                        skillsAmount++;
                    }
                    else
                    {
                        skills+=", "+query.value(0).toString();
                        skillsAmount++;
                    }
                }
                for(int i = 0; i<tableSkills.length(); i++)//считаем сколько навыков работник ввел
                    if(tableSkills[i]==",")
                        commasInSkills++;

                //QString tableSkillsMas[commasInSkills+1];//создаем массив размерностью равному количеству навыков введенных работником
                QVector<QString> tableSkillsMas(commasInSkills+1);


                for(int i=0; i<tableSkills.length(); i++)//заполняем массив введенных навыков
                {
                    if(tableSkills[i]!=",")
                    {
                        if (i>0)
                        {
                            if(tableSkills[i-1]!=","&&tableSkills[i]!=" ")
                                tableSkillsMas[curMasIndex]+=tableSkills[i];

                            else if(tableSkills[i-1]==","&&tableSkills[i]!=" ")
                                tableSkillsMas[curMasIndex]+=tableSkills[i];

                            else if(i<tableSkills.length()-1)
                                if(tableSkills[i-1]!=","&&tableSkills[i-1]!=" "&&tableSkills[i]==" "&&tableSkills[i+1]!=" "&&tableSkills[i+1]!=",")
                                    tableSkillsMas[curMasIndex]+=tableSkills[i];
                        }
                        else
                            tableSkillsMas[curMasIndex]+=tableSkills[i];
                    }
                    else
                        curMasIndex++;
                }

                query.prepare("SELECT line_employee_skills_id FROM employee_skills WHERE cv_id_skill = "+cv+" ORDER BY line_employee_skills_id");
                query.exec();
                //QString lineSkillMas[query.size()];//создаем массив размерностью равному количеству id строк навыков
                QVector<QString> lineSkillMas(query.size());

                int k=0;
                while (query.next())//заполняем массив значениями id строк навыков
                {
                    lineSkillMas[k]=query.value(0).toString();
                    k++;
                }

                if(skills!=tableSkills)//если содержимое навыков было изменено
                {
                    if(commasInSkills+1==skillsAmount)//если количество навыков не изменилось
                    {
                        for(int i = 0; i<skillsAmount; i++)//обновляем навыки в базе данных
                        {
                            query.prepare("UPDATE employee_skills SET skill = :skillValue WHERE line_employee_skills_id =:line_employee_skills_idValue;");
                            query.bindValue(":skillValue", tableSkillsMas[i]);
                            query.bindValue(":line_employee_skills_idValue", lineSkillMas[i]);
                            query.exec();
                        }
                    }
                    else if (commasInSkills+1<skillsAmount)//если количество навыков уменьшилось
                    {
                        int dif=skillsAmount-(commasInSkills+1);//получаем количество удаленных навыков
                        for(int i = 0; i<dif; i++)//удаляем навыки из базы данных
                        {
                            query.prepare("DELETE FROM employee_skills WHERE line_employee_skills_id =:line_employee_skills_idValue;");
                            query.bindValue(":line_employee_skills_idValue", lineSkillMas[skillsAmount-1-i]);
                            query.exec();
                        }
                        for(int i = 0; i<(commasInSkills+1); i++)//обновляем навыки в базе данных
                        {
                            query.prepare("UPDATE employee_skills SET skill = :skillValue WHERE line_employee_skills_id =:line_employee_skills_idValue;");
                            query.bindValue(":skillValue", tableSkillsMas[i]);
                            query.bindValue(":line_employee_skills_idValue", lineSkillMas[i]);
                            query.exec();
                        }
                    }
                    else if (commasInSkills+1>skillsAmount)//если количество навыков увеличилось
                    {
                        query.prepare("SELECT line_employee_skills_id FROM employee_skills ORDER BY line_employee_skills_id");
                        query.exec();
                        query.last();
                        int lastSkillsID=query.value(0).toInt();
                        for(int i = skillsAmount; i<commasInSkills+1; i++)//добавляем навыки в базу данных
                        {
                            query.prepare("INSERT INTO employee_skills (line_employee_skills_id, cv_id_skill, skill) "
                                          "VALUES (:line_employee_skills_idValue, :cv_id_skillValue, :skill)");
                            query.bindValue(":line_employee_skills_idValue", lastSkillsID + 1);
                            query.bindValue(":cv_id_skillValue", cv);
                            query.bindValue(":skill", tableSkillsMas[i]);
                            query.exec();
                            lastSkillsID++;
                        }
                        for(int i = 0; i<skillsAmount; i++)//обновляем навыки в базе данных
                        {
                            query.prepare("UPDATE employee_skills SET skill = :skillValue WHERE line_employee_skills_id =:line_employee_skills_idValue;");
                            query.bindValue(":skillValue", tableSkillsMas[i]);
                            query.bindValue(":line_employee_skills_idValue", lineSkillMas[i]);
                            query.exec();
                        }
                    }
                }
                currentRow++;
                if(i==ui->tableWidgetAdminEmployee->rowCount()-1)
                {
                    tableSkillsMas.clear();
                    lineSkillMas.clear();
                    QObject::disconnect(ui->tableWidgetAdminEmployee,nullptr,nullptr,nullptr);
                    ui->tableWidgetAdminEmployee->setEditTriggers(QAbstractItemView::NoEditTriggers);//запрещаем редактирование таблицы
                    ui->widgetShowForAdmin->show();
                    ui->pushButtonAdminDontSave->hide();
                    ui->pushButtonAdminSave->hide();
                    on_pushButtonShowEmployees_clicked();
                    ui->pushButtonBack->show();
                    ui->pushButtonAdmin->show();
                    ui->widgetDeleteVacancy->show();
                    QMessageBox::information(this, "Информация", "Данные успешно сохранены");//показываем информацию пользователю
                }
            }
        }
    }
    if(CompanyAdminOpened)//если открыт список компания для администратора
    {
        for(int i=0; i < ui->tableWidgetAdminCompanies->rowCount(); i++)//проверяем есть ли пустые ячейки в таблице
        {
            for(int j=0; j < ui->tableWidgetAdminCompanies->columnCount(); j++)
            {
                currentCell = ui->tableWidgetAdminCompanies->item(i,j)->text()==nullptr;
                if(currentCell)
                    cellsAny=true;
            }
        }
        if(!ui->tableWidgetAdminVacancies->isHidden())//если открыт список вакансий
        {
            for(int i=0; i < ui->tableWidgetAdminVacancies->rowCount(); i++)//проверяем правильно ли введены навыки
                if(CheckSkills(ui->tableWidgetAdminVacancies->item(i,3)->text()))
                    checkSkillsCommaAny=true;

            for(int i=0; i < ui->tableWidgetAdminVacancies->rowCount(); i++)//проверяем есть ли пустые ячейки в таблице
            {
                for(int j=0; j < ui->tableWidgetAdminVacancies->columnCount(); j++)
                {
                    currentCell = ui->tableWidgetAdminVacancies->item(i,j)->text()==nullptr;
                    if(currentCell)
                        cellsAny=true;
                }
            }
        }
        if(cellsAny)//выводим ошибки
            QMessageBox::warning(this, "Предупреждение", "Ячейки должны быть заполнены");

        else if(checkSkillsCommaAny)
            QMessageBox::warning(this, "Предупреждение", "Пример правильно введенной строки навыков: skill1, skill2, skill3");

        else if(!cellsAny&&!checkSkillsCommaAny)//если ошибок нет обновляем компании
        {
            QSqlQuery query;
            for(int i = 0; i < ui->tableWidgetAdminCompanies->rowCount(); i++)//обновляем компании
            {
                QString company_id=ui->tableWidgetAdminCompanies->item(i,0)->text();
                //QString company_id=QString::number(curCompany);
                query.prepare("UPDATE company SET description = :descriptionValue, name = :nameValue, field_of_activity = :field_of_activityValue WHERE company_id = :company_idValue;");
                query.bindValue(":descriptionValue", ui->tableWidgetAdminCompanies->item(i,2)->text());
                query.bindValue(":nameValue", ui->tableWidgetAdminCompanies->item(i,1)->text());
                query.bindValue(":field_of_activityValue", ui->tableWidgetAdminCompanies->item(i,3)->text());
                query.bindValue(":company_idValue", company_id);
                query.exec();
            }
            if(!ui->tableWidgetAdminVacancies->isHidden())//если таблица вакансий открыта
            {
                for(int i=0; i<ui->tableWidgetAdminVacancies->rowCount(); i++)//обновляем данные вакансий
                {
                    tableSkills=ui->tableWidgetAdminVacancies->item(i,3)->text();
                    skillsAmount=0;
                    skills="";
                    curMasIndex=0;
                    commasInSkills=0;

                    QString job_format,vacancy_id=ui->tableWidgetAdminVacancies->item(i,0)->text();
                    //обновляем вакансию
                    query.prepare("UPDATE vacancy SET profession = :professionValue, experience = :experienceValue, contacts = :contactsValue WHERE vacancy_id = :vacancy_idValue;");
                    query.bindValue(":professionValue", ui->tableWidgetAdminVacancies->item(i,1)->text());
                    query.bindValue(":experienceValue", ui->tableWidgetAdminVacancies->item(i,2)->text());
                    query.bindValue(":contactsValue", ui->tableWidgetAdminVacancies->item(i,7)->text());
                    query.bindValue(":vacancy_idValue", vacancy_id);
                    query.exec();

                    query.prepare("SELECT job_format FROM vacancy WHERE vacancy_id = "+vacancy_id+"");
                    query.exec();
                    query.first();
                    job_format=query.value(0).toString();

                    //обновляем формат работы
                    query.prepare("UPDATE job_format SET location = :locationValue, work_schedule = :work_scheduleValue WHERE job_format_id =:job_format_idValue;");
                    query.bindValue(":locationValue", ui->tableWidgetAdminVacancies->item(i,5)->text());
                    query.bindValue(":work_scheduleValue", ui->tableWidgetAdminVacancies->item(i,4)->text());
                    query.bindValue(":job_format_idValue", job_format);
                    query.exec();

                    query.prepare("SELECT skill FROM skills_for_vacancy WHERE vacancy_id="+vacancy_id+"");
                    query.exec();
                    while(query.next())//считаем сколько навыков было в базе данных
                    {
                        if(skillsAmount==0)//если навык первый то добавляем его без запятой перед ним
                        {
                            skills+=query.value(0).toString();
                            skillsAmount++;
                        }
                        else
                        {
                            skills+=", "+query.value(0).toString();
                            skillsAmount++;
                        }
                    }
                    for(int i = 0; i<tableSkills.length(); i++)//считаем сколько навыков введено
                        if(tableSkills[i]==",")
                            commasInSkills++;

                    //QString tableSkillsMas[commasInSkills+1];//создаем массив размерностью равному количеству введенных навыков
                    QVector<QString> tableSkillsMas(commasInSkills+1);



                    for(int i=0; i<tableSkills.length(); i++)//заполняем массив введенных навыков
                    {
                        if(tableSkills[i]!=",")
                        {
                            if (i>0)
                            {
                                if(tableSkills[i-1]!=","&&tableSkills[i]!=" ")
                                    tableSkillsMas[curMasIndex]+=tableSkills[i];

                                else if(tableSkills[i-1]==","&&tableSkills[i]!=" ")
                                    tableSkillsMas[curMasIndex]+=tableSkills[i];

                                else if(i<tableSkills.length()-1)
                                    if(tableSkills[i-1]!=","&&tableSkills[i-1]!=" "&&tableSkills[i]==" "&&tableSkills[i+1]!=" "&&tableSkills[i+1]!=",")
                                        tableSkillsMas[curMasIndex]+=tableSkills[i];
                            }
                            else
                                tableSkillsMas[curMasIndex]+=tableSkills[i];
                        }
                        else
                            curMasIndex++;
                    }

                    query.prepare("SELECT line_skills_for_vacancy_id FROM skills_for_vacancy WHERE vacancy_id = "+vacancy_id+" ORDER BY line_skills_for_vacancy_id");
                    query.exec();
                    //QString lineSkillMas[query.size()];//создаем массив размерностью равному количеству id строк навыков
                    QVector<QString> lineSkillMas(query.size());

                    int k=0;
                    while (query.next())//заполняем массив значениями id строк навыков
                    {
                        lineSkillMas[k]=query.value(0).toString();
                        k++;
                    }

                    if(skills!=tableSkills)//если содержимое навыков было изменено
                    {
                        if(commasInSkills+1==skillsAmount)//если количество навыков не изменилось
                        {
                            for(int i = 0; i<skillsAmount; i++)//обновляем навыки в базе данных
                            {
                                query.prepare("UPDATE skills_for_vacancy SET skill = :skillValue WHERE line_skills_for_vacancy_id =:line_skills_for_vacancy_idValue;");
                                query.bindValue(":skillValue", tableSkillsMas[i]);
                                query.bindValue(":line_skills_for_vacancy_idValue", lineSkillMas[i]);
                                query.exec();
                            }
                        }
                        else if (commasInSkills+1<skillsAmount)//если количество навыков уменьшилось
                        {
                            int dif=skillsAmount-(commasInSkills+1);//получаем количество удаленных навыков
                            for(int i = 0; i<dif; i++)//удаляем навыки из базы данных
                            {
                                query.prepare("DELETE FROM skills_for_vacancy WHERE line_skills_for_vacancy_id =:line_skills_for_vacancy_idValue;");
                                query.bindValue(":line_skills_for_vacancy_idValue", lineSkillMas[skillsAmount-1-i]);
                                query.exec();
                            }
                            for(int i = 0; i<(commasInSkills+1); i++)//обновляем навыки в базе данных
                            {
                                query.prepare("UPDATE skills_for_vacancy SET skill = :skillValue WHERE line_skills_for_vacancy_id =:line_skills_for_vacancy_idValue;");
                                query.bindValue(":skillValue", tableSkillsMas[i]);
                                query.bindValue(":line_skills_for_vacancy_idValue", lineSkillMas[i]);
                                query.exec();
                            }
                        }
                        else if (commasInSkills+1>skillsAmount)//если количество навыков увеличилось
                        {
                            query.prepare("SELECT line_skills_for_vacancy_id FROM skills_for_vacancy ORDER BY line_skills_for_vacancy_id");
                            query.exec();
                            query.last();
                            int lastSkillsID=query.value(0).toInt();
                            for(int i = skillsAmount; i<commasInSkills+1; i++)//добавляем навыки в базу данных
                            {
                                query.prepare("INSERT INTO skills_for_vacancy (line_skills_for_vacancy_id, vacancy_id, skill) "
                                              "VALUES (:line_skills_for_vacancy_idValue, :vacancy_idValue, :skillValue)");
                                query.bindValue(":line_skills_for_vacancy_idValue", lastSkillsID + 1);
                                query.bindValue(":vacancy_idValue", vacancy_id);
                                query.bindValue(":skillValue", tableSkillsMas[i]);
                                query.exec();
                                lastSkillsID++;
                            }
                            for(int i = 0; i<skillsAmount; i++)//обновляем навыки в базе данных
                            {
                                query.prepare("UPDATE skills_for_vacancy SET skill = :skillValue WHERE line_skills_for_vacancy_id =:line_skills_for_vacancy_idValue;");
                                query.bindValue(":skillValue", tableSkillsMas[i]);
                                query.bindValue(":line_skills_for_vacancy_idValue", lineSkillMas[i]);
                                query.exec();
                            }
                        }
                    }
                    lineSkillMas.clear();
                    tableSkillsMas.clear();
                }
                QObject::disconnect(ui->tableWidgetAdminVacancies,nullptr,nullptr,nullptr);
                ui->tableWidgetAdminVacancies->setEditTriggers(QAbstractItemView::NoEditTriggers);//запрещаем редактирование таблицы
                ui->tableWidgetAdminVacancies->resizeColumnsToContents();//изменяем размер столбцов таблицы под текст внутри

                ui->widgetDeleteVacancyAdmin->show();
            }

            QObject::disconnect(ui->tableWidgetAdminCompanies,nullptr,nullptr,nullptr);
            ui->tableWidgetAdminCompanies->setEditTriggers(QAbstractItemView::NoEditTriggers);//запрещаем редактирование таблицы
            ui->widgetShowForAdmin->show();//скрываем и показываем необходимые элементы интерфейса
            ui->pushButtonAdminDontSave->hide();
            ui->pushButtonAdminSave->hide();
            on_pushButtonShowCompanies_clicked();
            ui->pushButtonBack->show();
            ui->pushButtonAdmin->show();
            ui->widgetDeleteVacancy->show();
            ui->widgetAdminVacancies->show();
            QMessageBox::information(this, "Информация", "Данные успешно сохранены");//показываем информацию пользователю
        }
    }
}

void MainWindow::on_pushButtonAdmin_clicked()//включаем режим редактирования
{
    ui->widgetShowForAdmin->hide();//скрываем и показываем необходимые элементы интерфейса
    ui->pushButtonAdmin->hide();
    ui->pushButtonBack->hide();
    ui->pushButtonAdminDontSave->show();
    ui->pushButtonAdminSave->show();

    if(EmployeeAdminOpened)//если открыты работники
    {
        ui->tableWidgetAdminEmployee->setSelectionMode(QAbstractItemView::NoSelection);
        ui->tableWidgetAdminEmployee->setEditTriggers(QAbstractItemView::NoEditTriggers);

        QObject::connect(ui->tableWidgetAdminEmployee, &QTableWidget::clicked,//разрешаем редактирование
                         ui->tableWidgetAdminEmployee, [=](const QModelIndex& index) {

            if (ui->tableWidgetAdminEmployee->item(index.row(), index.column())->flags() & Qt::ItemIsEditable )
                ui->tableWidgetAdminEmployee->edit(index);
        });
        ui->widgetDeleteVacancy->hide();
    }
    else if(CompanyAdminOpened)
    {
        ui->tableWidgetAdminCompanies->setSelectionMode(QAbstractItemView::NoSelection);
        ui->tableWidgetAdminCompanies->setEditTriggers(QAbstractItemView::NoEditTriggers);

        ui->tableWidgetAdminVacancies->setSelectionMode(QAbstractItemView::NoSelection);
        ui->tableWidgetAdminVacancies->setEditTriggers(QAbstractItemView::NoEditTriggers);

        ui->widgetDeleteVacancyAdmin->hide();
        ui->widgetDeleteVacancy->hide();
        ui->widgetAdminVacancies->hide();

        QObject::connect(ui->tableWidgetAdminCompanies, &QTableWidget::clicked,//разрешаем редактирование
                         ui->tableWidgetAdminCompanies, [=](const QModelIndex& index) {

            if (ui->tableWidgetAdminCompanies->item(index.row(), index.column())->flags() & Qt::ItemIsEditable )
                ui->tableWidgetAdminCompanies->edit(index);
        });

        QObject::connect(ui->tableWidgetAdminVacancies, &QTableWidget::clicked,//разрешаем редактирование
                         ui->tableWidgetAdminVacancies, [=](const QModelIndex& index) {

            if (ui->tableWidgetAdminVacancies->item(index.row(), index.column())->flags() & Qt::ItemIsEditable )
                ui->tableWidgetAdminVacancies->edit(index);
        });
    }
}

void MainWindow::on_pushButtonAdminDontSave_clicked()//не сохранять изменения внесенные администратором
{
    ui->widgetShowForAdmin->show();
    if(EmployeeAdminOpened)
    {
        QObject::disconnect(ui->tableWidgetAdminEmployee,nullptr,nullptr,nullptr);//отключаем редактирование
        ui->tableWidgetAdminEmployee->setEditTriggers(QAbstractItemView::NoEditTriggers);
        on_pushButtonShowEmployees_clicked();
        ui->widgetDeleteVacancy->show();
    }
    if(CompanyAdminOpened)
    {
        QObject::disconnect(ui->tableWidgetAdminCompanies,nullptr,nullptr,nullptr);//отключаем редактирование
        ui->tableWidgetAdminCompanies->setEditTriggers(QAbstractItemView::NoEditTriggers);//запрещаем редактирование таблицы

        QObject::disconnect(ui->tableWidgetAdminVacancies,nullptr,nullptr,nullptr);//отключаем редактирование
        ui->tableWidgetAdminVacancies->setEditTriggers(QAbstractItemView::NoEditTriggers);//запрещаем редактирование таблицы
        on_pushButtonShowCompanies_clicked();
        ui->widgetDeleteVacancy->show();
        ui->widgetDeleteVacancyAdmin->show();
        ui->widgetAdminVacancies->show();
        if(!ui->tableWidgetAdminVacancies->isHidden())
        {
            ui->widgetDeleteVacancyAdmin->show();
            on_pushButtonAdminShowVacancy_clicked();
        }
    }
    ui->pushButtonAdmin->show();//скрываем и показываем необходимые элементы интерфейса
    ui->pushButtonBack->show();

    ui->pushButtonAdminDontSave->hide();
    ui->pushButtonAdminSave->hide();
}

void MainWindow::on_pushButtonShowCompanies_clicked()//показать администратору компании
{
    QSqlQuery query;
    int currentRow=0;
    query.prepare("SELECT * FROM company ORDER BY company_id");
    query.exec();
    if(query.size()>0)//если в бд есть данные о компаниях
    {
        CompanyAdminOpened=true;
        EmployeeAdminOpened=false;
        ui->pushButtonAdmin->show();//скрываем и показываем необходимые элементы интерфейса
        ui->widgetAdminVacancies->show();
        ui->widgetDeleteVacancy->show();
        if(query.size()!=0)
            ui->spinBoxVacanyNumber->setMaximum(query.size());
        else
            ui->spinBoxVacanyNumber->setMaximum(1);
        ui->spinBoxDeleteVacancy->setMaximum(query.size());
        ui->tableWidgetAdminCompanies->setRowCount(query.size());
        CreateCells(query.size(),ui->tableWidgetAdminCompanies);
        ui->tableWidgetAdminEmployee->hide();
        ui->tableWidgetAdminCompanies->show();
        QObject::disconnect(ui->tableWidgetAdminCompanies,nullptr,nullptr,nullptr);//отключаем редактирование
        ui->tableWidgetAdminCompanies->setEditTriggers(QAbstractItemView::NoEditTriggers);//запрещаем редактирование таблицы
        ui->pushButtonDeleteVacancy->setText("Удалить компанию №");

        for(int row=0; row<ui->tableWidgetAdminCompanies->rowCount(); row++){
            for(int col=0; col< ui->tableWidgetAdminCompanies->columnCount(); col++){
                auto item=new QTableWidgetItem;
                if(col==0 || col==4 || col==5)
                    item->setFlags(item->flags() &  ~Qt::ItemIsEditable); //нередактируемые столбцы
                ui->tableWidgetAdminCompanies->setItem(row, col, item);
            }
        }

        while (query.next())
        {
            ui->tableWidgetAdminCompanies->item(currentRow,0)->setText(query.value(0).toString());//заполняем таблицу
            ui->tableWidgetAdminCompanies->item(currentRow,1)->setText(query.value(2).toString());
            ui->tableWidgetAdminCompanies->item(currentRow,2)->setText(query.value(1).toString());
            ui->tableWidgetAdminCompanies->item(currentRow,3)->setText(query.value(3).toString());
            ui->tableWidgetAdminCompanies->item(currentRow,4)->setText(query.value(4).toString());
            ui->tableWidgetAdminCompanies->item(currentRow,5)->setText(query.value(5).toString());
            currentRow++;
        }
        ui->tableWidgetAdminCompanies->resizeColumnsToContents();//изменяем размер столбцов таблицы под текст внутри
    }
    else
        QMessageBox::information(this, "Информация", "Данных о компаниях нет");
}

void MainWindow::on_pushButtonAdminShowVacancy_clicked()//показать вакансии администратору
{
    int lineNumber=ui->spinBoxVacanyNumber->value();
    QSqlQuery query, querySkills,qwAll;
    QString skills="",currentVacancyID, company_id;
    int skillsAmount=0,rowsCount=0,rows=0;
    company_id=ui->tableWidgetAdminCompanies->item(lineNumber-1,0)->text();
    query=db->exec("SELECT company_id FROM vacancy");
    while (query.next())
    {
        if(query.value(0).toString()==company_id)
            rows++;
    }

    ui->tableWidgetAdminVacancies->setRowCount(rows);
    CreateCells(rows,ui->tableWidgetAdminVacancies);
    qwAll=db->exec("SELECT * FROM vacancy INNER JOIN job_format ON job_format_id=job_format WHERE company_id="+company_id+" ORDER BY vacancy_id");

    for(int row=0; row<ui->tableWidgetAdminVacancies->rowCount(); row++){
        for(int col=0; col< ui->tableWidgetAdminVacancies->columnCount(); col++){
            auto item=new QTableWidgetItem;
            if(col==0 || col==6)
                item->setFlags(item->flags() &  ~Qt::ItemIsEditable); //нередактируемые столбцы
            ui->tableWidgetAdminVacancies->setItem(row, col, item);
        }
    }

    while (qwAll.next())
    {
        currentVacancyID = qwAll.value(0).toString();
        querySkills=db->exec("SELECT skill FROM skills_for_vacancy WHERE vacancy_id = "+currentVacancyID+" ORDER BY line_skills_for_vacancy_id");
        while(querySkills.next())
        {
            if(skillsAmount==0)//если навык первый то добавляем его без запятой перед ним
            {
                skills+=querySkills.value(0).toString();
                skillsAmount++;
            }
            else
                skills+=", "+querySkills.value(0).toString();//заполняем переменную навыками
        }
        ui->tableWidgetAdminVacancies->item(rowsCount,0)->setText(qwAll.value(0).toString());//заполняем таблицу
        ui->tableWidgetAdminVacancies->item(rowsCount,1)->setText(qwAll.value(3).toString());
        ui->tableWidgetAdminVacancies->item(rowsCount,2)->setText(qwAll.value(6).toString());
        ui->tableWidgetAdminVacancies->item(rowsCount,3)->setText(skills);
        ui->tableWidgetAdminVacancies->item(rowsCount,4)->setText(qwAll.value(9).toString());
        ui->tableWidgetAdminVacancies->item(rowsCount,5)->setText(qwAll.value(8).toString());
        ui->tableWidgetAdminVacancies->item(rowsCount,6)->setText(qwAll.value(1).toString());
        ui->tableWidgetAdminVacancies->item(rowsCount,7)->setText(qwAll.value(5).toString());
        skills="";
        skillsAmount=0;
        rowsCount++;
    }
    if(rowsCount==0)
        QMessageBox::information(this, "Информация", "Данные о всех вакансиях компании удалены");//показываем информацию пользователю
    else
    {
        QObject::disconnect(ui->tableWidgetAdminVacancies,nullptr,nullptr,nullptr);//отключаем редактирование
        ui->tableWidgetAdminVacancies->setEditTriggers(QAbstractItemView::NoEditTriggers);//запрещаем редактирование таблицы
        ui->tableWidgetAdminVacancies->resizeColumnsToContents();//изменяем размер столбцов таблицы под текст внутри
        ui->tableWidgetAdminVacancies->show();//скрываем и показываем необходимые элементы интерфейса
        ui->widgetDeleteVacancyAdmin->show();
        curCompany=ui->spinBoxVacanyNumber->value();
    }
}

void MainWindow::on_pushButtonDeleteVacancyAdmin_clicked()//удалить вакансию открытую для администратора
{
    QSqlQuery query;
    int lineNumber;
    QString  vacancy_id, job_format;
    lineNumber = ui->spinBoxDeleteVacancyAdmin->value();//получаем номер вакансии для удаления

    if(ui->tableWidgetAdminVacancies->rowCount()<lineNumber)//если номера вакансии для удаления нет выдаем ошибку
        QMessageBox::warning(this, "Предупреждение", "Вакансии с таким номером нет.");
    else
    {
        vacancy_id=ui->tableWidgetAdminVacancies->item(lineNumber-1,0)->text();
        query.prepare("SELECT job_format FROM vacancy WHERE vacancy_id="+vacancy_id+"");
        query.exec();
        query.first();
        job_format=query.value(0).toString();

        //удаление навыков конкретной вакансии из базы данных
        query.prepare("DELETE FROM skills_for_vacancy WHERE vacancy_id ="+vacancy_id+"");
        query.exec();

        //удаление вакансии из базы данных
        query.prepare("DELETE FROM vacancy WHERE vacancy_id ="+vacancy_id+"");
        query.exec();

        //удаление формата работы из базы данных
        query.prepare("DELETE FROM job_format WHERE job_format_id ="+job_format+"");
        query.exec();

        QMessageBox::information(this, "Удаление вакансии", "Вакансия удалена.");//показываем информацию пользователю
        if(curCompany!=0)
        {
            ui->spinBoxVacanyNumber->setValue(curCompany);
            on_pushButtonAdminShowVacancy_clicked();
        }
    }
}
