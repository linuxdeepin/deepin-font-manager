#ifndef DFONTPREVIEWPROXYMODEL_H
#define DFONTPREVIEWPROXYMODEL_H

#include <QStandardItemModel>
#include <QSortFilterProxyModel>

class DFontPreviewProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit DFontPreviewProxyModel(QObject *parent = nullptr);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void setFilterGroup(int filterGroup);

    void setUseSystemFilter(bool useSystemFilter);

    bool isCustomFilterAcceptsRow(const QModelIndex &modelIndex) const;

    bool isFontNameContainsPattern(QString fontName) const;

    void setFilterFontNamePattern(const QString &pattern);

//    void setEditStatus(bool editStatus);

//    bool getEditStatus() const;

signals:
    void onFilterFinishRowCountChangedInt(unsigned int bShow) const;

private:
    int m_filterGroup;
    bool m_useSystemFilter;
//    bool m_editStatus;
    QString m_fontNamePattern;
};

#endif  // DFONTPREVIEWPROXYMODEL_H
