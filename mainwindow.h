#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

//диалоговое окно предпросмотра конфига
#include "previewdialog.h"

//для отладки
#include <QDebug>

//класс для работы с текстовыми переменными
#include <QString>

//класс для работы со списками
#include <QList>

//для вывода различный сообщений (об ошибках или успешных операциях)
#include <QMessageBox>

//для диалологовых окон "Открыть", "Сохранить" и т.д.
#include <QFileDialog>

//для получения QCoreApplication::applicationDirPath() папки исполняемой программы
#include <QCoreApplication>

//для чтения, записи файлов
#include <QFile>

//для работы с текстовыми потоками (для чтения и записи файлов построчно)
#include <QTextStream>

//для метода getFileNameOrValue
#include <QPair>

//для перехвата события закрытия окна
#include <QCloseEvent>



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    /**
     * @brief closeEvent перегружаем событие на закрытие окна
     * @param event
     */
    void closeEvent(QCloseEvent *event);

    /**
     * @brief confirmExit выводит диалоговое окно с подтверждением выхода
     * @return
     */
    bool confirmExit();

    /**
     * @brief on_About_QT_Action_triggered слот для осного меню программы (About > About Qt)
     */
    void on_About_Qt_Action_triggered();

    /**
     * @brief on_openConfigAtion_triggered слот для осного меню программы (File > Открыть OpenVPN Config)
     */
    void on_openConfigAtion_triggered();

    /**
     * @brief on_openConfigFilePushButton_released слот на кнопочку Открыть конфиг
     */
    void on_openConfigFilePushButton_released();

    /**
     * @brief openConfig открытие конфига. Вынесли в отдельный метод, потому что используется несколько раз
     */
    void openConfig();



    void on_ca_typeComboBox_currentIndexChanged(int index = 0);

    void on_cert_typeComboBox_currentIndexChanged(int index = 0);

    void on_key_typeComboBox_currentIndexChanged(int index = 0);

    void on_tls_auth_typeComboBox_currentIndexChanged(int index = 0);

    void on_createNewConfigAction_triggered();

    void on_exitAction_triggered();

    void on_exitPushButton_released();

    void on_savePushButton_released();

    void on_previewPushButton_released();

    void on_saveAsAction_triggered();

    void on_saveConfigAction_triggered();

private:
    Ui::MainWindow *ui;

    /**
     * @brief m_currentConfigFileName для хранения имени файла конфигурции OpenVPN
     */
    QString m_currentConfigFileName="";

    /**
      * @brief m_configExtenionList допустимые расшерения файла конфига в списке
      */
    const QStringList m_configExtenionList = {"*.conf", "*.ovpn"};

    /**
     * @brief m_currentConfigFileLines текущий файл конфига по строкам
     */
    QStringList m_currentConfigFileLines;

    /**
     * @brief m_lastLineIndex последний индекс редктируемой строки, для того что бы в конфиге было все по порядку
     */
    int m_lastLineIndex = 0;

    /**
     * @brief isOpenVPNConfig метод для проверки подлинности файла конфига
     * @param fileLineList ссылка на
     * @return
     */
    bool isOpenVPNConfig(QStringList const &fileLineList);

    /**
     * @brief parseConfig метод для парсинга конфига из файла
     */
    void parseConfig();

    /**
     * @brief getConfigFromForm заполняет m_currentConfigFileLines параметрами с формы
     */
    void getConfigFromForm();

    /**
     * @brief findInLineList находит строку с начальным параметром
     * @param var начальный параметр
     * @param singleParametr если true - значит параметр должен быть 1 в строчке
     * @return индекс строки в глобальном списке или -1 если не нашло
     */
    int findInLineList(QString key, bool singleParametr=true);

    /**
     * @brief getValueFromLineList возвращает значение по ключу
     * @param key
     * @return значение или пустая строка
     */
    QString getValueFromLineList (QString key);

    /**
     * @brief getFileNameOrValue возвращает тип сертификата/ключа в паре, где первое значение - QString текст, второе значение 0 - не создано, 1 - файл, 2 - текст
     * @param val название сертификата, ключа
     * @return первое значение - QString текст, второе значение 0 - не создано, 1 - файл, 2 - текст
     */
    QPair<QString, int> getFileNameOrValue(QString val);

    /**
     * @brief getTagsValue возвращает значение в теге, если в файле конфигурации вшиты ключи/сертификаты
     * @param tag название тега
     * @return текст который между тегами <tag> и закрывающим тегом </tag>
     */
    QString getTagsValue(QString tag);

    /**
     * @brief addCheckBoxData если true - то добавляет параметр в конфиг, если false - удалает
     * @param parametrName имя параметра
     * @param isChecked
     */
    void addCheckBoxData (QString parametrName, bool isChecked);

    /**
     * @brief addKeyValueData устанавливает в конфиг параметр ключ значение, если значение пустое, то удаляет параметр
     * @param key ключ
     * @param value значение
     */
    void addKeyValueData (QString key, QString value);

    /**
     * @brief addCertKey устанавливает в конфиг имя файла сертификата/ключа или текст сертификата/ключа
     * @param certKeyName название сертификата или ключа
     * @param value значение
     * @param type тип (0 - удалить, 1 - файл, 2 - текст (между тегами)
     */
    void addCertKey (QString certKeyName, QString value, int type);

    /**
     * @brief resetForm сбрасывает форму.
     */
    void clearForm();

    /**
     * @brief removeTag удаляет из конфига все что в теге <tag> ... </tag>
     * @param tag
     */
    void removeTag(QString tag);


    /**
     * @brief addTag добавляет в конфиг тег со значением <tag>value</tag>
     * @param tag тэг
     * @param value значение
     */
    void addTag(QString tag, QString value);

    /**
     * @brief saveConfigToFile сохраняет конфиг в файл m_currentConfigFileName
     * @param fileName имя файла
     */
    void saveConfigToFile(QString fileName);

};
#endif // MAINWINDOW_H
