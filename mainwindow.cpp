#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("File: /etc/openvpn/client.conf");

    on_ca_typeComboBox_currentIndexChanged(0); //скрываем все у ca
    on_cert_typeComboBox_currentIndexChanged(0); //скрываем все у cert
    on_key_typeComboBox_currentIndexChanged(0); //скрываем все у key
    on_tls_auth_typeComboBox_currentIndexChanged(0); //скрываем все у tls-auth

    this->setWindowTitle("New config file");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (confirmExit()){
        event->accept();
    }
    else {
        event->ignore();
    }
}

bool MainWindow::confirmExit()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Подтвердите выход из приложения");
    msgBox.setIcon(QMessageBox::Question);

    msgBox.setText("При закрытии приложения все несохраненные данные будут безвозвратно потеряны.<br><b>Выйти из приложения</b>?");
    QAbstractButton* pButtonYes = msgBox.addButton("Да", QMessageBox::YesRole); msgBox.addButton("Нет", QMessageBox::NoRole);

    msgBox.exec();

    if (msgBox.clickedButton()==pButtonYes) {
        return true;
    }
    else {
        return false;
    }
}



void MainWindow::on_About_Qt_Action_triggered()
{
    QMessageBox::aboutQt(this, "About Qt");
}

void MainWindow::on_openConfigAtion_triggered()
{
    openConfig();
}

void MainWindow::on_openConfigFilePushButton_released()
{
    openConfig();
}

void MainWindow::openConfig()
{

    //для начала сохраняем имя файла во временную переменну. После проверки является ли файл конфигом, перенемем в свойства класса
    QString tempFileName = QFileDialog::getOpenFileName(this, //родительский виджет
                                                        "Открыть файл конфигурации OpenVPN", //заголовок диалогового окна
                                                        QCoreApplication::applicationDirPath(), //по умолчанию выставляем путь там где храниться программка
                                                        QString("OpenVPN config (%1)").arg(m_configExtenionList.join(", "))); //задаем фильр файлов

    if (tempFileName.trimmed() == ""){ //если сделали отмену, или закрыли диалог, то возвращаемся
        return ;
    }



    QFile tempConfigFile(tempFileName);

    //если не смогли открыть файл для чтения в текстовом режиме, тогда выводим ошибку и выходим с метода
    if (!tempConfigFile.open(QIODevice::ReadOnly)){
        QMessageBox::critical(this, "Ошибка!", QString("Не смогли открыть файл конфигурации <b>%1</b>").arg(tempFileName));
        return;
    }

    //если файл открыли, то читаем его построчно и каждую строку записываем в QStringList
    QStringList tempFileLineList;
    QTextStream textStream(&tempConfigFile);

    while (!textStream.atEnd()){
        tempFileLineList.append(textStream.readLine());
    }

    //проверяем является ли файл OpenVPN клиентским конфигом
    if (isOpenVPNConfig(tempFileLineList)==false){
        QMessageBox::warning(this, "Внимание", QString("Файл <b>%1</b> не является файлом конфига OpenVPN").arg(tempFileName));
        return;
    }

    clearForm();

    //если файл успешно загружен, то парсим файл
    m_currentConfigFileLines = tempFileLineList;
    m_currentConfigFileName = tempFileName;

    ui->fileNameLineEdit->setText(m_currentConfigFileName);
    this->setWindowTitle(m_currentConfigFileName);

    parseConfig();
}

bool MainWindow::isOpenVPNConfig(const QStringList &fileLineList)
{
    //проверяем на наличие ключевых слов в начале строки "client" "remote", если этого нет, то это точно не конфиг OpenVPN
    QString tempStrClient="client";
    QString tempStrRemote = "remote";
    bool hasClientStr = false;
    bool hasRemoteStr = false;

    for (int i=0; i<fileLineList.count(); i++){
        QString currentLine = fileLineList.value(i).simplified(); //создаем копию строки, удаляем двойные пробелы, убираем пробелы в начале и в конце
        if (currentLine.left(tempStrClient.length()) == tempStrClient){
            hasClientStr = true;
        }
        else if (currentLine.left(tempStrRemote.length()) == tempStrRemote){
            hasRemoteStr = true;
        }

        if (hasClientStr && hasRemoteStr){ //если есть ключевые слова, то возвращаем true
            return true;
        }
    }

    return false; //если не нашли в цикле ключевых слов, то возвращаем false
}

void MainWindow::parseConfig()
{
    QString value;
    int index;

    //ищем параметр dev
    value = getValueFromLineList("dev");
    if (value!=""){
        int index = ui->devComboBox->findText(value.toUpper());
        index = index == -1 ? 0 : index;
        ui->devComboBox->setCurrentIndex(index);

    }

    //ищем параметр proto
    value = getValueFromLineList("proto");
    if (value!=""){
        int index = ui->protoComboBox->findText(value.toUpper());
        index = index == -1 ? 0 : index;
        ui->protoComboBox->setCurrentIndex(index);
    }

    //remote
    value = getValueFromLineList("remote");
    if (value!=""){
        QStringList hostPortList = value.split(" ");
        ui->remoteHostLineEdit->setText(hostPortList.value(0));
        ui->remotePortLineEdit->setText(QString::number(static_cast<quint16>(hostPortList.value(1).toInt())));
    }

    //resolv-retry
    value = getValueFromLineList("resolv-retry");
    if (value!=""){
        ui->resolv_retryLineEdit->setText(value);
    }

    //nobind
    index = findInLineList("nobind");
    ui->nobindCheckBox->setChecked(index >= 0 ? true : false);

    //user
    value = getValueFromLineList("user");
    if (value!=""){
        ui->userLineEdit->setText(value);
    }

    //group
    value = getValueFromLineList("group");
    if (value!=""){
        ui->groupLineEdit->setText(value);
    }

    //persist-key
    index = findInLineList("persist-key");
    ui->persist_keyCcheckBox->setChecked(index >= 0 ? true : false);

    //persist-tun
    index = findInLineList("persist-tun");
    ui->persist_tunCheckBox->setChecked(index >= 0 ? true : false);

    //remote-cert-tls
    value = getValueFromLineList("remote-cert-tls");
    if (value!=""){
        ui->remote_cert_tlsLineEdit->setText(value);
    }

    //http-proxy
    value = getValueFromLineList("http-proxy");
    if (value!=""){
        QStringList hostPortList = value.split(" ");
        ui->http_proxy_hostLineEdit->setText(hostPortList.value(0));
        ui->http_proxy_portLineEdit->setText(QString::number(static_cast<quint16>(hostPortList.value(1).toInt())));
    }

    //http-proxy-timeout
    value = getValueFromLineList("http-proxy-timeout");
    if (value!=""){
        ui->http_proxy_timeoutLineEdit->setText(value);
    }

    //http-proxy-retry
    value = getValueFromLineList("http-proxy-retry");
    if (value!=""){
        ui->http_proxy_retryLineEdit->setText(value);
    }

    //mute-replay-warnings
    index = findInLineList("mute-replay-warnings");
    ui->mute_replay_warningsCheckBox->setChecked(index >= 0 ? true : false);

    //tls-client
    index = findInLineList("tls-client");
    ui->tls_clientCheckBox->setChecked(index >= 0 ? true : false);

    //comp-lzo
    index = findInLineList("comp-lzo");
    ui->comp_lzoCheckBox->setChecked(index >= 0 ? true : false);

    //verb
    value = getValueFromLineList("verb");
    if (value!=""){
        ui->verbLineEdit->setText(value);
    }

    //cipher
    value = getValueFromLineList("cipher");
    if (value!=""){
        int index = ui->cipherComboBox->findText(value.toUpper());
        index = index == -1 ? 0 : index;
        ui->cipherComboBox->setCurrentIndex(index);
    }

    //auth
    value = getValueFromLineList("auth");
    if (value!=""){
        ui->authLineEdit->setText(value);
    }

    //key-direction
    value = getValueFromLineList("key-direction");
    if (value!=""){
        ui->key_directionCheckBox->setChecked(value.toInt() == 1 ? true : false);
    }

    //================= берем ключи сертификаты ==================
    QPair <QString, int> certKeyValue;

    //берем CA (корневой сертификат)
    certKeyValue = getFileNameOrValue("ca");
    ui->ca_typeComboBox->setCurrentIndex(certKeyValue.second);
    if (certKeyValue.second == 1){ //значит файл
        ui->ca_fileLineEdit->setText(certKeyValue.first);
    }
    else if (certKeyValue.second == 2){
        ui->ca_textPlainTextEdit->setPlainText(certKeyValue.first);
    }

    //берем CERT (открытый ключ клиента)
    certKeyValue = getFileNameOrValue("cert");
    ui->cert_typeComboBox->setCurrentIndex(certKeyValue.second);
    if (certKeyValue.second == 1){ //значит файл
        ui->cert_fileLineEdit->setText(certKeyValue.first);
    }
    else if (certKeyValue.second == 2){
        ui->cert_textPlainTextEdit->setPlainText(certKeyValue.first);
    }

    //берем KEY (Закрытый ключ клиента)
    certKeyValue = getFileNameOrValue("key");
    ui->key_typeComboBox->setCurrentIndex(certKeyValue.second);
    if (certKeyValue.second == 1){ //значит файл
        ui->key_fileLineEdit->setText(certKeyValue.first);
    }
    else if (certKeyValue.second == 2){
        ui->key_textPlainTextEdit->setPlainText(certKeyValue.first);
    }

    //берем TLS-AUTH (Закрытый ключ клиента)
    certKeyValue = getFileNameOrValue("tls-auth");
    ui->tls_auth_typeComboBox->setCurrentIndex(certKeyValue.second);
    if (certKeyValue.second == 1){ //значит файл
        ui->tls_auth_fileLineEdit->setText(certKeyValue.first);
    }
    else if (certKeyValue.second == 2){
        ui->tls_auth_textPlainTextEdit->setPlainText(certKeyValue.first);
    }


}

void MainWindow::getConfigFromForm()
{
    m_lastLineIndex=0;

    //используем этот метод что бы установить параметр client (на случай если это новый файл);
    addCheckBoxData("client", true);

    //устанавливаем dev
    addKeyValueData("dev", ui->devComboBox->currentIndex()==0 ? "" : ui->devComboBox->currentText().toLower());

    //усиаеавомваем proto
    addKeyValueData("proto", ui->protoComboBox->currentIndex()==0 ? "" : ui->protoComboBox->currentText().toLower());

    //устанавливаем remote
    addKeyValueData("remote", (ui->remoteHostLineEdit->text().trimmed()!="" && ui->remotePortLineEdit->text().trimmed()!="")
                    ? (ui->remoteHostLineEdit->text().trimmed()+" " + QString::number(ui->remotePortLineEdit->text().trimmed().toInt()))
                    : "");

    //resolv-retry
    addKeyValueData("resolv-retry", ui->resolv_retryLineEdit->text());

    //nobind
    addCheckBoxData("nobind", ui->nobindCheckBox->isChecked());

    //user
    addKeyValueData("user", ui->userLineEdit->text());

    //group
    addKeyValueData("group", ui->groupLineEdit->text());

    //persist-key
    addCheckBoxData("persist-key", ui->persist_keyCcheckBox->isChecked());

    //persist-tun
    addCheckBoxData("persist-tun", ui->persist_tunCheckBox->isChecked());

    //remote-cert-tls
    addKeyValueData("remote-cert-tls", ui->remote_cert_tlsLineEdit->text());

    //устанавливаем http-proxy
    addKeyValueData("http-proxy", (ui->http_proxy_hostLineEdit->text().trimmed()!="" && ui->http_proxy_portLineEdit->text().trimmed()!="")
                    ? (ui->http_proxy_hostLineEdit->text().trimmed()+" " + QString::number(ui->http_proxy_portLineEdit->text().trimmed().toInt()))
                    : "");

    //http-proxy-timeout
    addKeyValueData("http-proxy-timeout", ui->http_proxy_timeoutLineEdit->text());

    //http-proxy-retry
    addKeyValueData("http-proxy-retry", ui->http_proxy_retryLineEdit->text());

    //mute-replay-warnings
    addCheckBoxData("mute-replay-warnings", ui->mute_replay_warningsCheckBox->isChecked());

    //tls-client
    addCheckBoxData("tls-client", ui->tls_clientCheckBox->isChecked());

    //comp-lzo
    addCheckBoxData("comp-lzo", ui->comp_lzoCheckBox->isChecked());

    //verb
    addKeyValueData("verb", ui->verbLineEdit->text());

    //cipher
    addKeyValueData("cipher", ui->cipherComboBox->currentIndex()==0 ? "" : ui->cipherComboBox->currentText());

    //auth
    addKeyValueData("auth", ui->authLineEdit->text());

    //key-direction
    addKeyValueData("key-direction", ui->key_directionCheckBox->isChecked() ? "1" : "");

    //================ сертификаты / ключи =====================
    QString value;
    //ca
    if (ui->ca_typeComboBox->currentIndex() == 1) { //файл
        value = ui->ca_fileLineEdit->text();
    }
    else if (ui->ca_typeComboBox->currentIndex() == 2) { //текст
        value = ui->ca_textPlainTextEdit->toPlainText();
    }
    else {
        value = "";
    }
    addCertKey("ca", value, ui->ca_typeComboBox->currentIndex());

    //cert
    if (ui->cert_typeComboBox->currentIndex() == 1) { //файл
        value = ui->cert_fileLineEdit->text();
    }
    else if (ui->cert_typeComboBox->currentIndex() == 2) { //текст
        value = ui->cert_textPlainTextEdit->toPlainText();
    }
    else {
        value = "";
    }
    addCertKey("cert", value, ui->cert_typeComboBox->currentIndex());

    //key
    if (ui->key_typeComboBox->currentIndex() == 1) { //файл
        value = ui->key_fileLineEdit->text();
    }
    else if (ui->key_typeComboBox->currentIndex() == 2) { //текст
        value = ui->key_textPlainTextEdit->toPlainText();
    }
    else {
        value = "";
    }
    addCertKey("key", value, ui->key_typeComboBox->currentIndex());

    //tls-auth
    if (ui->tls_auth_typeComboBox->currentIndex() == 1) { //файл
        value = ui->tls_auth_fileLineEdit->text();
    }
    else if (ui->tls_auth_typeComboBox->currentIndex() == 2) { //текст
        value = ui->tls_auth_textPlainTextEdit->toPlainText();
    }
    else {
        value = "";
    }
    addCertKey("tls-auth", value, ui->tls_auth_typeComboBox->currentIndex());


}

int MainWindow::findInLineList(QString key, bool singleParametr)
{
    key = key.trimmed();
    if (singleParametr==false){ //добавляем пробел в конец, что бы не вернуло часть слова
        key+=" ";
    }
    int keyLength = key.length();
    for (int i=0; i<m_currentConfigFileLines.count(); i++){
        QString currentLine = m_currentConfigFileLines.value(i).simplified();
        if (singleParametr){
            if (key == currentLine){
                return i;
            }
        }
        else if (currentLine.left(keyLength) == key){
            return i;
        }
    }
    return -1;
}

QString MainWindow::getValueFromLineList(QString key)
{
    key = key.trimmed() + " ";
    int index = findInLineList(key, false);
    if (index == -1){
        return "";
    }
    else {
        QString line = m_currentConfigFileLines.value(index).simplified();
        return line.mid(key.length()).trimmed();
    }
}

QPair<QString, int> MainWindow::getFileNameOrValue(QString val)
{
    QPair<QString, int> keyCertPair;
    keyCertPair.first = "";
    keyCertPair.second = 0;
    QString tempValue = getValueFromLineList(val).trimmed();
    if (tempValue!=""){ //значит это файл (индекс 1 в комбобоксе)
        keyCertPair.first = tempValue;
        keyCertPair.second = 1;
    }
    else {
        tempValue = getTagsValue(val);
        if (tempValue!=""){ //значит ключ или сертификат заключен в тегах
            keyCertPair.first = tempValue;
            keyCertPair.second = 2;
        }
    }
    return keyCertPair;

}

QString MainWindow::getTagsValue(QString tag)
{
    QString startTag = QString("<%1>").arg(tag);
    QString endTag = QString("</%1>").arg(tag);
    int tagStartLineIndex = findInLineList(startTag);
    int tagEndLineIndex = findInLineList(endTag);

    if (tagStartLineIndex == -1 || tagEndLineIndex == -1 || tagEndLineIndex < tagStartLineIndex){ //если нет стартующего или закрывающего тега, то возвращаем пустое значение
        return "";
    }
    else {
        return m_currentConfigFileLines.mid(tagStartLineIndex+1, (tagEndLineIndex-tagStartLineIndex-1)).join("\n");
    }


}

void MainWindow::addCheckBoxData(QString parametrName, bool isChecked)
{
    parametrName=parametrName.trimmed();
    if (parametrName == ""){
        return;
    }

    qDebug() << Q_FUNC_INFO << "parametr" << parametrName << "isChecked" << isChecked << "m_lastLineIndex" << m_lastLineIndex;

    int index = findInLineList(parametrName);
    if (index >= 0 && isChecked == false){ //если элемент есть, но его не нужно, то удаляем строку
        m_currentConfigFileLines.removeAt(index);
        m_lastLineIndex = index;
    }
    else if (index == -1 && isChecked == true){ //если элемента нет, а он нужен, то вставляем после m_lastLineIndex
        m_currentConfigFileLines.insert(m_lastLineIndex, parametrName);
        m_lastLineIndex++;
    }

}

void MainWindow::addKeyValueData(QString key, QString value)
{
    key = key.trimmed();
    value = value.simplified();

    qDebug() << Q_FUNC_INFO << "key" << key << "value" << value;

    int index = findInLineList(key, false);
    if (index >= 0 && value==""){ //значит нужно удалить
        m_currentConfigFileLines.removeAt(index);
        m_lastLineIndex = index;
    }
    else if (index == -1 && value != ""){ //значит нужно добавить
        QString tempLine = key + " " + value;
        m_currentConfigFileLines.insert(m_lastLineIndex, tempLine);
        m_lastLineIndex++;
    }
    else if (index >=0 && value !="") { //значит нужно заменить
        QString tempLine = key + " " + value;
        m_currentConfigFileLines[index] = tempLine;
        m_lastLineIndex = index+1;
    }
}

void MainWindow::addCertKey(QString certKeyName, QString value, int type)
{

    certKeyName = certKeyName.trimmed();
    value = value.trimmed();

    if (type == 0 || value == ""){ //если тип "не указано" или значение пустое, значит надо удалить
        //удаляем файл
        addKeyValueData(certKeyName, "");

        //удаляем в тегах
        removeTag(certKeyName);
    }
    else if (type == 1){ //значит надо файл
        removeTag(certKeyName); //удаляем, если есть
        addKeyValueData(certKeyName, value);
    }
    else if (type == 2){ //значит сертификат или ключ надо добавить в теге
        removeTag(certKeyName);
        addKeyValueData(certKeyName, ""); //удаляем, если есть в конфиге файл
        addTag(certKeyName, value);
    }

}

void MainWindow::clearForm()
{
    QList<QLineEdit*> lineEditList = this->findChildren<QLineEdit*>();
    for (int i=0; i<lineEditList.count(); i++){
        lineEditList.value(i)->clear();
    }

    QList<QPlainTextEdit*> plainTextEditList = this->findChildren<QPlainTextEdit*>();
    for (int i=0; i<plainTextEditList.count(); i++){
        plainTextEditList.value(i)->clear();
    }

    QList <QComboBox*> comboBoxList = this->findChildren<QComboBox*>();
    for (int i=0; i<comboBoxList.count(); i++){
        comboBoxList.value(i)->setCurrentIndex(0);
    }

    QList <QCheckBox*> checkBoxList = this->findChildren<QCheckBox*>();
    for (int i=0; i<checkBoxList.count(); i++){
        checkBoxList.value(i)->setChecked(false);
    }



    this->setWindowTitle("New config file");
}

void MainWindow::removeTag(QString tag)
{
    QString startTag = QString("<%1>").arg(tag);
    QString endTag = QString("</%1>").arg(tag);

    int tagStartLineIndex = findInLineList(startTag);
    int tagEndLineIndex = findInLineList(endTag);

    if (tagStartLineIndex>=0 && tagEndLineIndex>=0 && tagStartLineIndex<tagEndLineIndex){
        m_currentConfigFileLines = m_currentConfigFileLines.mid(0, tagStartLineIndex) + m_currentConfigFileLines.mid(tagEndLineIndex+1);
    }
}

void MainWindow::addTag(QString tag, QString value)
{
    tag = tag.trimmed();
    value = value.trimmed();

    if (tag == "" || value == ""){
        return;
    }

    QStringList tagList;
    tagList
            << QString ("<%1>").arg(tag)
            << value
            << QString ("</%1>").arg(tag);


    for (int i=0; i<tagList.count(); i++){
        //m_currentConfigFileLines.insert(m_lastLineIndex, tagList.value(i));
        m_currentConfigFileLines.append(tagList.value(i));
        //m_lastLineIndex++;
    }
    //m_lastLineIndex++;

    qDebug() << "tag" << tag << "all line" << m_currentConfigFileLines.count() << "m_lastLineIndex" << m_lastLineIndex;

}

void MainWindow::saveConfigToFile(QString fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)){
        file.write(m_currentConfigFileLines.join("\n").toUtf8());
        this->setWindowTitle(fileName);
        ui->fileNameLineEdit->setText(fileName);
        QMessageBox::information(this, "Success", "Успешно сохранили файл настроек");
    }
    else {
        QMessageBox::critical(this, "Faile", QString("Не смогли открыть файл <b>%1</b>.<br>Снимите защиту от записи и попробуйте снова").arg(fileName));
    }
}

void MainWindow::on_ca_typeComboBox_currentIndexChanged(int index)
{
    if (index == 0){ // 0 индекс значит не выбрано, скрываем все
        ui->ca_fileLabel->setHidden(true);
        ui->ca_fileLineEdit->setHidden(true);
        ui->ca_textPlainTextEdit->setHidden(true);
    }
    else if (index == 1){ // 1 - значит выбран файл
        ui->ca_fileLabel->setHidden(false);
        ui->ca_fileLineEdit->setHidden(false);
        ui->ca_textPlainTextEdit->setHidden(true);
    }
    else if (index == 2) { // 2 - значит текст
        ui->ca_fileLabel->setHidden(true);
        ui->ca_fileLineEdit->setHidden(true);
        ui->ca_textPlainTextEdit->setHidden(false);
    }
}

void MainWindow::on_cert_typeComboBox_currentIndexChanged(int index)
{
    if (index == 0){ // 0 индекс значит не выбрано, скрываем все
        ui->cert_fileLabel->setHidden(true);
        ui->cert_fileLineEdit->setHidden(true);
        ui->cert_textPlainTextEdit->setHidden(true);
    }
    else if (index == 1){ // 1 - значит выбран файл
        ui->cert_fileLabel->setHidden(false);
        ui->cert_fileLineEdit->setHidden(false);
        ui->cert_textPlainTextEdit->setHidden(true);
    }
    else if (index == 2) { // 2 - значит текст
        ui->cert_fileLabel->setHidden(true);
        ui->cert_fileLineEdit->setHidden(true);
        ui->cert_textPlainTextEdit->setHidden(false);
    }
}

void MainWindow::on_key_typeComboBox_currentIndexChanged(int index)
{
    if (index == 0){ // 0 индекс значит не выбрано, скрываем все
        ui->key_fileLabel->setHidden(true);
        ui->key_fileLineEdit->setHidden(true);
        ui->key_textPlainTextEdit->setHidden(true);
    }
    else if (index == 1){ // 1 - значит выбран файл
        ui->key_fileLabel->setHidden(false);
        ui->key_fileLineEdit->setHidden(false);
        ui->key_textPlainTextEdit->setHidden(true);
    }
    else if (index == 2) { // 2 - значит текст
        ui->key_fileLabel->setHidden(true);
        ui->key_fileLineEdit->setHidden(true);
        ui->key_textPlainTextEdit->setHidden(false);
    }
}

void MainWindow::on_tls_auth_typeComboBox_currentIndexChanged(int index)
{
    if (index == 0){ // 0 индекс значит не выбрано, скрываем все
        ui->tls_auth_fileLabel->setHidden(true);
        ui->tls_auth_fileLineEdit->setHidden(true);
        ui->tls_auth_textPlainTextEdit->setHidden(true);
    }
    else if (index == 1){ // 1 - значит выбран файл
        ui->tls_auth_fileLabel->setHidden(false);
        ui->tls_auth_fileLineEdit->setHidden(false);
        ui->tls_auth_textPlainTextEdit->setHidden(true);
    }
    else if (index == 2) { // 2 - значит текст
        ui->tls_auth_fileLabel->setHidden(true);
        ui->tls_auth_fileLineEdit->setHidden(true);
        ui->tls_auth_textPlainTextEdit->setHidden(false);
    }
}

void MainWindow::on_createNewConfigAction_triggered()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Подтвердите действие");
    msgBox.setIcon(QMessageBox::Question);

    msgBox.setText("После создания нового файла конфигурации все не сохраненные данные будут уничтожены<br><b>Продолжить</b>?");
    QAbstractButton* pButtonYes = msgBox.addButton("Да", QMessageBox::YesRole); msgBox.addButton("Нет", QMessageBox::NoRole);

    msgBox.exec();

    if (msgBox.clickedButton()==pButtonYes) {
        m_currentConfigFileName="";
        m_currentConfigFileLines.clear();
        clearForm();
    }
}

void MainWindow::on_exitAction_triggered()
{
    if (confirmExit()){
        QCoreApplication::quit();
    }
}

void MainWindow::on_exitPushButton_released()
{
    if (confirmExit()){
        QCoreApplication::quit();
    }
}

void MainWindow::on_savePushButton_released()
{
    if (m_currentConfigFileName.trimmed() == ""){
       on_saveAsAction_triggered();
    }
    else {
        saveConfigToFile(m_currentConfigFileName);
    }
}

void MainWindow::on_previewPushButton_released()
{
    getConfigFromForm();
    PreviewDialog preview(this);
    preview.setPreviewText(m_currentConfigFileLines.join("\n"));
    preview.exec();
}

void MainWindow::on_saveAsAction_triggered()
{
    //для начала сохраняем имя файла во временную переменну. После проверки является ли файл конфигом, перенемем в свойства класса
    QString tempFileName = QFileDialog::getSaveFileName(this, //родительский виджет
                                                        "Сохранить файл конфигурации OpenVPN", //заголовок диалогового окна
                                                        QCoreApplication::applicationDirPath(), //по умолчанию выставляем путь там где храниться программка
                                                        QString("OpenVPN config (%1)").arg(m_configExtenionList.join(", "))); //задаем фильр файлов
    tempFileName = tempFileName.trimmed();
    if (tempFileName == ""){
        return;
    }
    saveConfigToFile(tempFileName);

}

void MainWindow::on_saveConfigAction_triggered()
{
    //одни и тоже дейсвие что у кнопки, что из под меню
    on_savePushButton_released();
}
