#ifndef DFMXMLWRAPPER_H
#define DFMXMLWRAPPER_H

#include <QtXml>

typedef QMap<QString, QString> QSTRING_MAP;
typedef QMap<QString, QString>::iterator QSTRING_MAP_ITER;
typedef QVector<QString> QSTRING_VECTOR;
typedef QVector<QString>::Iterator QSTRING_VECTOR_ITER;

/*************************************************************************
 <Class>         DFMXmlWrapper
 <Description>   xml文件管理类
 <Author>
 <Note>          null
*************************************************************************/

class DFMXmlWrapper
{
public:
    DFMXmlWrapper();

    static QString m_fontConfigFilePath;

public:

    //新建xml文件
    static bool createXmlFile(const QString &fileName,
                              const QString &rootName,
                              const QString &version = "1.0",
                              const QString &encoding = "gb2312",
                              const QString &standalone = "yes");

    //新建fontconfig配置文件
    static bool createFontConfigFile(const QString &xmlFilePath);

    //删除文件
    static bool deleteXmlFile(const QString &fileName);


    //重命名xml文件
    static bool renameXmlFile(const QString &fileName,
                              const QString &newName);

    // 根据节点名获取节点元素
    static bool getNodeByName(QDomElement &rootEle,
                              const QString &nodeName,
                              QDomElement &node);

    //增加只有文本的节点
    static bool addNode_Text(const QString &fileName,
                             const QString &parentNodeName,
                             const QString &nodeName,
                             const QString &nodeText);

    //增加节点
    static bool addNodesWithText(const QString &fileName,
                                 const QString &parentNodeName,
                                 const QStringList &nodeNameList,
                                 const QList<QSTRING_MAP> &nodeAttributeList,
                                 const QString &lastNodeText);

    static bool addNodesWithTextList(const QString &fileName,
                                     const QString &parentNodeName,
                                     const QStringList &nodeNameList,
                                     const QList<QSTRING_MAP> &nodeAttributeList,
                                     const QStringList &lastNodeTextList);

    static bool addPatternNodesWithText(const QString &fileName,
                                        const QString &parentNodeName,
                                        const QString &lastNodeText);

    static bool addPatternNodesWithTextList(const QString &fileName,
                                            const QString &parentNodeName,
                                            const QStringList &lastNodeTextList);

    //@brief  增加包含文本与属性的节点
    static bool addNode_All(const QString &fileName,
                            const QString &parentNodeName,
                            const QString &nodeName,
                            const QString &nodeText,
                            QSTRING_MAP &attMap);

    //删除节点
    static bool deleteNodeWithText(const QString &fileName,
                                   const QString &nodeName,
                                   const QString &nodeText);

    //删除节点
    static bool deleteNodeWithTextList(const QString &fileName,
                                       const QString &nodeName,
                                       const QStringList &nodeTextList);

    //修改节点文本
    static bool modifyNode_Text(const QString &fileName,
                                const QString &nodeName,
                                const QString &nodeText);

    //修改节点属性
    static bool modifyNode_Attribute(const QString &fileName,
                                     const QString &nodeName,
                                     QSTRING_MAP &attMap);

    //查询节点文本
    static bool queryNode_Text(const QString &fileName,
                               const QString &nodeName,
                               QString &nodeText);

    //查询节点属性
    static bool queryNode_Attribute(const QString &fileName,
                                    const QString &nodeName,
                                    const QString &attName,
                                    QString &attValue);

    //查询所有子节点文本
    static bool queryAllChildNodes_Text(const QString &fileName,
                                        const QString &nodeName,
                                        QSTRING_VECTOR &textVector);

    // 查询所有子节点文本
    static bool queryAllChildNodes_Text(const QString &fileName,
                                        const QString &nodeName,
                                        QStringList &textList);


    //查询所有禁用字体文件路径列表
    static QStringList getFontConfigDisableFontPathList();
};

#endif // QXMLWRAPPER_H
