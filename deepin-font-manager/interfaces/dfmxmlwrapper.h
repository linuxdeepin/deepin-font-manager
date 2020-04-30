#ifndef DFMXMLWRAPPER_H
#define DFMXMLWRAPPER_H

#include <QtXml>
#include <QString>
#include <QMap>
#include <QVector>

typedef QMap<QString, QString> QSTRING_MAP;
typedef QMap<QString, QString>::iterator QSTRING_MAP_ITER;
typedef QVector<QString> QSTRING_VECTOR;
typedef QVector<QString>::Iterator QSTRING_VECTOR_ITER;


class DFMXmlWrapper
{
public:
    DFMXmlWrapper();

    static QString m_fontConfigFilePath;

public:

    /**
    *  @brief  新建xml文件
    *  @param[in]  fileName 文件名
    *  @param[in]  rootName 根节点名
    *  @param[in]  version xml版本
    *  @param[in]  encoding xml编码（字符集）
    *  @param[in]  standalone 是否需要依赖其他的文件
    *  @retval true 表示成功
    *  @retval false 表示失败
    */
    static bool createXmlFile(const QString &fileName,
                              const QString &rootName,
                              const QString &version = "1.0",
                              const QString &encoding = "gb2312",
                              const QString &standalone = "yes");

    /**
    *  @brief  新建fontconfig配置文件
    *  @param[in]  xmlFilePath 文件路径
    *  @retval true 表示成功
    *  @retval false 表示失败
    */
    static bool createFontConfigFile(const QString &xmlFilePath);

    /**
    *  @brief  删除文件
    *  @param[in]  fileName 文件名
    *  @retval true 表示成功
    *  @retval false 表示失败
    */
    static bool deleteXmlFile(const QString &fileName);


    /**
    *  @brief  重命名xml文件
    *  @param[in]  fileName 文件名
    *  @param[in]  newName 新名称
    *  @retval true 表示成功
    *  @retval false 表示失败
    */
    static bool renameXmlFile(const QString &fileName,
                              const QString &newName);

    /**
    *  @brief  根据节点名获取节点元素
    *  @param[in]  rootEle 根元素
    *  @param[in]  nodeName 节点名
    *  @param[out]  node 节点
    *  @retval true 表示成功
    *  @retval false 表示失败
    */
    static bool getNodeByName(QDomElement &rootEle,
                              const QString &nodeName,
                              QDomElement &node);

    /**
    *  @brief  增加只有文本的节点
    *  @param[in]  fileName 文件名
    *  @param[in]  parentNodeName 父节点名
    *  @param[in]  nodeName 节点名
    *  @param[in]  nodeText 节点文本
    *  @retval true 表示成功
    *  @retval false 表示失败
    */
    static bool addNode_Text(const QString &fileName,
                             const QString &parentNodeName,
                             const QString &nodeName,
                             const QString &nodeText);

    /**
    *  @brief  增加节点
    *  @param[in]  fileName 文件名
    *  @param[in]  parentNodeName 父节点名
    *  @param[in]  nodePropertyList 节点名列表(将依次作为子节点添加到父节点上，
    * 　　　比如父节点名叫"parentNode", nodeMapList包含"nodeA", "nodeB", "nodeC"三个节点)，
    * 　　　则节点全部加入后变成：
    *      <parentNode>
    *         <nodeA>
    *           <nodeB property="value">
    *              <nodeC>
    *                 nodeText
    *              </nodeC>
    *           </nodeB>
    *         </nodeA>
    *      </parentNode>
    *  @param[in]  nodeAttributeList 节点的属性/值map列表
    *  @param[in]  lastNodeText 用于填充在最后一个节点的文本
    *  @retval true 表示成功
    *  @retval false 表示失败
    */
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

    /**
    *  @brief  增加包含文本与属性的节点
    *  @param[in]  fileName 文件名
    *  @param[in]  parentNodeName 父节点名
    *  @param[in]  nodeName 节点名
    *  @param[in]  nodeText 节点文本
    *  @param[in]  attMap 节点属性
    *  @retval true 表示成功
    *  @retval false 表示失败
    */
    static bool addNode_All(const QString &fileName,
                            const QString &parentNodeName,
                            const QString &nodeName,
                            const QString &nodeText,
                            QSTRING_MAP &attMap);

    /**
    *  @brief  删除节点
    *  @param[in]  fileName 文件名
    *  @param[in]  nodeName 节点名
    *  @param[in]  nodeText 节点包含的文本
    *  @retval true 表示成功
    *  @retval false 表示失败
    */
    static bool deleteNodeWithText(const QString &fileName,
                                   const QString &nodeName,
                                   const QString &nodeText);

    /**
    *  @brief  删除节点
    *  @param[in]  fileName 文件名
    *  @param[in]  nodeName 节点名
    *  @param[in]  nodeText 节点包含的文本
    *  @retval true 表示成功
    *  @retval false 表示失败
    */
    static bool deleteNodeWithTextList(const QString &fileName,
                                       const QString &nodeName,
                                       const QStringList &nodeTextList);

    /**
    *  @brief  修改节点文本
    *  @param[in]  fileName 文件名
    *  @param[in]  nodeName 节点名
    *  @param[in]  nodeText 节点文本
    *  @retval true 表示成功
    *  @retval false 表示失败
    */
    static bool modifyNode_Text(const QString &fileName,
                                const QString &nodeName,
                                const QString &nodeText);

    /**
    *  @brief  修改节点属性
    *  @param[in]  fileName 文件名
    *  @param[in]  nodeName 节点名
    *  @param[in]  attMap 节点属性
    *  @retval true 表示成功
    *  @retval false 表示失败
    */
    static bool modifyNode_Attribute(const QString &fileName,
                                     const QString &nodeName,
                                     QSTRING_MAP &attMap);

    /**
    *  @brief  查询节点文本
    *  @param[in]  fileName 文件名
    *  @param[in]  nodeName 节点名
    *  @param[out]  nodeText 节点文本
    *  @retval true 表示成功
    *  @retval false 表示失败
    */
    static bool queryNode_Text(const QString &fileName,
                               const QString &nodeName,
                               QString &nodeText);

    /**
    *  @brief  查询节点属性
    *  @param[in]  fileName 文件名
    *  @param[in]  nodeName 节点名
    *  @param[in]  attName 节点属性名
    *  @param[out]  attValue 节点属性值
    *  @retval true 表示成功
    *  @retval false 表示失败
    */
    static bool queryNode_Attribute(const QString &fileName,
                                    const QString &nodeName,
                                    const QString &attName,
                                    QString &attValue);

    /**
    *  @brief  查询所有子节点文本
    *  @param[in]  fileName 文件名
    *  @param[in]  nodeName 节点名
    *  @param[out]  textVector 子节点文本
    *  @retval true 表示成功
    *  @retval false 表示失败
    */
    static bool queryAllChildNodes_Text(const QString &fileName,
                                        const QString &nodeName,
                                        QSTRING_VECTOR &textVector);

    /**
    *  @brief  查询所有子节点文本
    *  @param[in]  fileName 文件名
    *  @param[in]  nodeName 节点名
    *  @param[out]  textList 子节点文本
    *  @retval true 表示成功
    *  @retval false 表示失败
    */
    static bool queryAllChildNodes_Text(const QString &fileName,
                                        const QString &nodeName,
                                        QStringList &textList);


    //查询所有禁用字体文件路径列表
    static QStringList getFontConfigDisableFontPathList();
};

#endif // QXMLWRAPPER_H
