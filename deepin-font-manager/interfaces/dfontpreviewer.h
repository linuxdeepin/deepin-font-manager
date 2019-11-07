#ifndef DFONTPREVIEWER_H
#define DFONTPREVIEWER_H

#include <DFrame>

DWIDGET_USE_NAMESPACE

class DFontPreviewer : public QWidget
{
    Q_OBJECT
public:
    explicit DFontPreviewer(QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *event) override;

    void InitData();
    void InitConnections();
    void setPreviewFontPath(const QString font);
signals:
    void previewFontChanged();
public slots:
    void onPreviewFontChanged();
private:
    QString m_fontPath;
    QStringList m_previewTexts;
};

#endif // DFONTPREVIEWER_H
