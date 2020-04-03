#include "dfmxmlwrapper.h"
#include "globaldef.h"

QString DFMXmlWrapper::m_fontConfigFilePath = QDir::homePath() + "/.config/fontconfig/conf.d/" + FTM_REJECT_FONT_CONF_FILENAME;

DFMXmlWrapper::DFMXmlWrapper()
{
}

bool DFMXmlWrapper::createXmlFile(const QString &fileName,
                                  const QString &rootName,
                                  const QString &version,
                                  const QString &encoding,
                                  const QString &standalone)
{
    QFile file(fileName);

    // 只写方式打开，并清空以前的信息
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    QDomDocument doc;
    //添加处理指令（声明）
    QDomProcessingInstruction instruction;
    QString data;
    data = "version=\"" + version + "\" encoding=\"" + encoding + "\" standalone=\"" + standalone + "\"";
    instruction = doc.createProcessingInstruction("xml", data);
    doc.appendChild(instruction);
    QDomElement root = doc.createElement(rootName);
    doc.appendChild(root); //添加根元素

    QTextStream out(&file);
    // 将文档保存到文件，4为子元素缩进字符数
    doc.save(out, 4);
    file.close();
    return true;
}

bool DFMXmlWrapper::createFontConfigFile(QString xmlFilePath)
{
    QFile file(xmlFilePath);
    if (file.exists()) {
        qDebug() << "file is already exist!";
        return true;
    }

    if (!file.open(QFile::WriteOnly | QFile::Text)) { // 只写模式打开文件
        qDebug() << QString("Cannot write file %1(%2).").arg(xmlFilePath).arg(file.errorString());
        return false;
    }

    QXmlStreamWriter writer(&file);
    // 自动格式化
    writer.setAutoFormatting(true);
    // 开始文档（XML 声明）
    writer.writeStartDocument("1.0");
    // DTD
    writer.writeDTD(QString::fromLocal8Bit("<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">"));

    writer.writeStartElement("fontconfig");  // 开始根元素 <fontconfig>
    writer.writeStartElement("selectfont");  // 开始子元素 <selectfont>
    writer.writeStartElement("rejectfont");  // 开始子元素 <rejectfont>
    writer.writeStartElement("pattern");
    writer.writeStartElement("patelt");
    writer.writeAttribute("name", "file");
    writer.writeTextElement("string", "[place your file path here!]");
    writer.writeEndElement();  // 结束子元素 </string>
    writer.writeEndElement();  // 结束子元素 </patelt>
    writer.writeEndElement();  // 结束子元素 </pattern>
    writer.writeEndElement();  // 结束子元素 </rejectfont>
    writer.writeEndElement();  // 结束子元素 </selectfont>
    writer.writeEndElement();  // 结束根元素 </fontconfig>
    writer.writeEndDocument();

    file.close();

    return true;
}

bool DFMXmlWrapper::deleteXmlFile(const QString &fileName)
{
    if (fileName.isEmpty()) {
        return false;
    }

    QFile file(fileName);
    if (!file.setPermissions(QFile::WriteOwner)) // 修改文件属性
        return false;
    if (!QFile::remove(fileName)) // 删除文件
        return false;

    return true;
}

bool DFMXmlWrapper::renameXmlFile(const QString &fileName,
                                  const QString &newName)
{
    return QFile::rename(fileName, newName);
}

bool DFMXmlWrapper::getNodeByName(QDomElement &rootEle,
                                  const QString &nodeName,
                                  QDomElement &node)
{
    if (nodeName == rootEle.tagName()) { // 若为根节点，则返回
        node = rootEle;
        return true;
    }

    QDomElement ele = rootEle;

    for (ele = rootEle.firstChildElement(); ele.isElement(); ele = ele.nextSiblingElement()) {
        //递归处理子节点，获取节点
        if (getNodeByName(ele, nodeName, node)) {
            return true;
        }
    }
    return false;
}


bool DFMXmlWrapper::addNode_Text(const QString &fileName,
                                 const QString &parentNodeName,
                                 const QString &nodeName,
                                 const QString &nodeText)
{
    if (fileName.isEmpty()) { // 文件名为空
        return false;
    }

    // 新建QDomDocument类对象，它代表一个XML文档
    QDomDocument doc;
    // 建立指向“fileName”文件的QFile对象
    QFile file(fileName);
    // 以只读方式打开
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // 将文件内容读到doc中
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    // 关闭文件
    file.close();

    QDomElement rootEle = doc.documentElement();
    QDomElement parentNode;
    getNodeByName(rootEle, parentNodeName, parentNode);

    // 添加元素及其文本
    QDomElement childEle = doc.createElement(nodeName);
    QDomText text;
    text = doc.createTextNode(nodeText);
    childEle.appendChild(text);
    parentNode.appendChild(childEle);

    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return false;
    }

    //输出到文件
    QTextStream out(&file);
    doc.save(out, 4); // 将文档保存到文件，4为子元素缩进字符数
    file.close();

    return true;
}

bool DFMXmlWrapper::addNodesWithText(const QString &fileName,
                                     const QString &parentNodeName,
                                     const QStringList &nodeNameList,
                                     const QList<QSTRING_MAP> &nodeAttributeList,
                                     const QString &lastNodeText)
{
    if (fileName.isEmpty()) { // 文件名为空
        return false;
    }

    // 新建QDomDocument类对象，它代表一个XML文档
    QDomDocument doc;
    // 建立指向“fileName”文件的QFile对象
    QFile file(fileName);
    // 以只读方式打开
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // 将文件内容读到doc中
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }

    file.close();

    QDomElement rootEle = doc.documentElement();
    QDomElement parentNode;
    getNodeByName(rootEle, parentNodeName, parentNode);

    QDomElement currParentNode = parentNode;
    for (int i = 0; i < nodeNameList.size(); i++) {

        QString nodeName = nodeNameList.at(i);
        QSTRING_MAP attributeMap = nodeAttributeList.at(i);

        // 添加元素
        QDomElement childEle = doc.createElement(nodeName);
        currParentNode.appendChild(childEle);
        currParentNode = childEle;

        if (!attributeMap.empty()) {
            QString attribute = attributeMap.keys().first();
            QString value = attributeMap.value(attribute);

            childEle.setAttribute(attribute, value);
        }

        if (nodeNameList.size() - 1 == i) {
            QDomText text;
            text = doc.createTextNode(lastNodeText);
            childEle.appendChild(text);
        }
    }

    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return false;
    }

    //输出到文件
    QTextStream out(&file);
    // 将文档保存到文件，4为子元素缩进字符数
    doc.save(out, 4);
    file.close();

    return true;
}

bool DFMXmlWrapper::addPatternNodesWithText(const QString &fileName,
                                            const QString &parentNodeName,
                                            const QString &lastNodeText)
{
    QStringList nodeNameList;
    nodeNameList << "pattern" << "patelt" << "string";
    QList<QMap<QString, QString>> attributeList;
    QMap<QString, QString> map1;
    QMap<QString, QString> map2;
    map2.insert("name", "file");
    QMap<QString, QString> map3;
    attributeList.push_back(map1);
    attributeList.push_back(map2);
    attributeList.push_back(map3);

    return DFMXmlWrapper::addNodesWithText(fileName, parentNodeName, nodeNameList, attributeList, lastNodeText);
}

bool DFMXmlWrapper::addNode_All(const QString &fileName,
                                const QString &parentNodeName,
                                const QString &nodeName,
                                const QString &nodeText,
                                QSTRING_MAP &attMap)
{
    if (fileName.isEmpty()) {
        return false;
    }

    // 建立指向“fileName”文件的QFile对象
    QFile file(fileName);
    // 以只读方式打开
    if (!file.open(QIODevice::ReadOnly))
        return false;

    // 新建QDomDocument类对象，它代表一个XML文档
    QDomDocument doc;
    // 将文件内容读到doc中
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    QDomElement rootEle = doc.documentElement();
    QDomElement parentNode;
    getNodeByName(rootEle, parentNodeName, parentNode);

    // 添加元素及其文本
    QDomElement childEle = doc.createElement(nodeName);
    QDomText text;
    text = doc.createTextNode(nodeText);
    childEle.appendChild(text);  // 添加文本

    // 添加属性
    QSTRING_MAP_ITER it;
    for ( it = attMap.begin(); it != attMap.end(); ++it ) {
        // 属性名
        QDomAttr att = doc.createAttribute(it.key());
        // 属性值
        att.setValue(it.value());
        childEle.setAttributeNode(att);
    }

    parentNode.appendChild(childEle);

    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return false;
    }

    //输出到文件
    QTextStream out(&file);
    doc.save(out, 4); // 将文档保存到文件，4为子元素缩进字符数
    file.close();

    return true;
}

bool DFMXmlWrapper::deleteNodeWithText(const QString &fileName,
                                       const QString &nodeName,
                                       const QString &nodeText)
{
    if (fileName.isEmpty()) {
        return false;
    }

    QFile file(fileName);
    //打开文件
    if (!file.open(QFile::ReadOnly))
        return false;

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    QDomElement rootEle = doc.documentElement();
    QDomElement nodeEle;
    getNodeByName(rootEle, nodeName, nodeEle);

    // 假如是根节点
    if (rootEle == nodeEle) {
        return false;
    }

    //根据节点包含的文本匹配到的节点，删除节点及其元素
    QDomNode removeNode;
    QDomNodeList list = nodeEle.parentNode().childNodes();
    for (int i = 0; i < list.count(); i++) {
        QDomNode node = list.at(i);
        if (nodeEle.isElement()) {
            if (node.toElement().text() == nodeText) {
                removeNode = node;
                break;
            }
        }
    }

    if (removeNode.isElement()) {
        QDomNode parentNode = removeNode.parentNode();
        if (parentNode.isElement()) {
            parentNode.removeChild(removeNode);
        } else {
            qDebug() << "delete node failed!" << endl;
            return false;
        }
    } else {
        qDebug() << "delete node failed!" << endl;
    }

    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return false;
    }

    //输出到文件
    QTextStream out_stream(&file);
    doc.save(out_stream, 4); //缩进4格
    file.close();

    return true;
}

bool DFMXmlWrapper::modifyNode_Text(const QString &fileName,
                                    const QString &nodeName,
                                    const QString &nodeText)
{
    if (fileName.isEmpty()) {
        return false;
    }

    //打开文件
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        return false;
    }

    //删除一个一级子节点及其元素，外层节点删除内层节点于此相同
    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    QDomElement rootEle = doc.documentElement();
    QDomElement nodeEle;
    getNodeByName(rootEle, nodeName, nodeEle);

    if (nodeEle.isElement()) {
        //标签之间的内容作为节点的子节点出现，得到原来的子节点
        QDomNode oldnode = nodeEle.firstChild();
        //用提供的value值来设置子节点的内容
        nodeEle.firstChild().setNodeValue(nodeText);

        //值修改过后, 调用节点的replaceChild方法实现修改功能
        QDomNode newnode = nodeEle.firstChild();
        nodeEle.replaceChild(newnode, oldnode);
    }

    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return false;
    }

    //输出到文件
    QTextStream out_stream(&file);
    //缩进4格
    doc.save(out_stream, 4);
    file.close();

    return true;
}

bool DFMXmlWrapper::modifyNode_Attribute(const QString &fileName,
                                         const QString &nodeName,
                                         QSTRING_MAP &attMap)
{
    if (fileName.isEmpty()) {
        return false;
    }

    //打开文件
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        return false;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    QDomElement rootEle = doc.documentElement();
    QDomElement nodeEle;
    getNodeByName(rootEle, nodeName, nodeEle);

    if (nodeEle.isElement()) {
        QSTRING_MAP_ITER it;
        //遍历map
        for ( it = attMap.begin(); it != attMap.end(); ++it ) {
            QDomAttr att = nodeEle.attributeNode(it.key());
            // 删除之前的属性及其值
            nodeEle.removeAttribute(it.key());
            // 更新属性值
            att.setValue(it.value());
            nodeEle.setAttributeNode(att);
        }
    }

    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return false;
    }

    //输出到文件
    QTextStream out_stream(&file);
    //缩进4格
    doc.save(out_stream, 4);
    file.close();
    return true;
}

bool DFMXmlWrapper::queryNode_Text(const QString &fileName,
                                   const QString &nodeName,
                                   QString &nodeText)
{
    if (fileName.isEmpty()) {
        return false;
    }

    // 新建QDomDocument类对象，它代表一个XML文档
    QDomDocument doc;
    // 建立指向“fileName”文件的QFile对象
    QFile file(fileName);
    // 以只读方式打开
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // 将文件内容读到doc中
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    QDomElement rootEle = doc.documentElement();
    QDomElement node;
    getNodeByName(rootEle, nodeName, node);

    if (node.isElement()) {
        nodeText = node.text();
        return true;
    } else {
        return false;
    }
}

bool DFMXmlWrapper::queryNode_Attribute(const QString &fileName,
                                        const QString &nodeName,
                                        const QString &attName,
                                        QString &attValue)
{
    if (fileName.isEmpty()) {
        return false;
    }

    // 新建QDomDocument类对象，它代表一个XML文档
    QDomDocument doc;
    // 建立指向“fileName”文件的QFile对象
    QFile file(fileName);
    // 以只读方式打开
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // 将文件内容读到doc中
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    //根元素
    QDomElement rootEle = doc.documentElement();
    QDomElement node;
    getNodeByName(rootEle, nodeName, node);

    if (node.isElement()) {
        attValue = node.attribute(attName);
        return true;
    } else {
        return false;
    }
}

bool DFMXmlWrapper::queryAllChildNodes_Text(const QString &fileName,
                                            const QString &nodeName,
                                            QSTRING_VECTOR &textVector)
{
    if (fileName.isEmpty()) {
        return false;
    }

    // 新建QDomDocument类对象，它代表一个XML文档
    QDomDocument doc;
    // 建立指向“fileName”文件的QFile对象
    QFile file(fileName);
    // 以只读方式打开
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // 将文件内容读到doc中
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    // 关闭文件
    file.close();

    //根元素
    QDomElement rootEle = doc.documentElement();
    QDomElement node;
    getNodeByName(rootEle, nodeName, node);

    QDomNodeList list = node.childNodes();
    for (int i = 0; i < list.count(); i++) {
        QDomNode n = list.at(i);
        if (node.isElement()) {
            textVector.insert(i, n.toElement().text());
        }
    }
    return true;
}

bool DFMXmlWrapper::queryAllChildNodes_Text(const QString &fileName,
                                            const QString &nodeName,
                                            QStringList &textList)
{
    if (fileName.isEmpty()) {
        return false;
    }

    // 新建QDomDocument类对象，它代表一个XML文档
    QDomDocument doc;
    // 建立指向“fileName”文件的QFile对象
    QFile file(fileName);
    // 以只读方式打开
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // 将文件内容读到doc中
    if (!doc.setContent(&file)) {
        file.close();
        file.remove();
        return false;
    }
    file.close();

    //根元素
    QDomElement rootEle = doc.documentElement();
    QDomElement node;
    getNodeByName(rootEle, nodeName, node);

    QDomNodeList list = node.childNodes();
    for (int i = 0; i < list.count(); i++) {
        QDomNode n = list.at(i);
        if (node.isElement()) {
            textList << n.toElement().text();
        }
    }

    return true;
}

QStringList DFMXmlWrapper::getFontConfigDisableFontPathList()
{
    QStringList strDisableFontPathList;
    QString fontConfigFilePath = DFMXmlWrapper::m_fontConfigFilePath;
    DFMXmlWrapper::queryAllChildNodes_Text(fontConfigFilePath, "rejectfont", strDisableFontPathList);

    return strDisableFontPathList;
}
