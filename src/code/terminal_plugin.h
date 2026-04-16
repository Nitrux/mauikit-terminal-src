#pragma once
#include <QDir>
#include <QLibraryInfo>
#include <QQmlExtensionPlugin>

class TerminalPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override;

private:
    void initializeEngine(QQmlEngine *engine, const char *uri) override;
    QUrl componentUrl(const QString &fileName) const;

    QString resolveFileUrl(const QString &filePath) const
    {
        const QString qmlModulePath = QDir(QLibraryInfo::path(QLibraryInfo::QmlImportsPath)).filePath(QStringLiteral("org/mauikit/terminal"));
        return QUrl::fromLocalFile(QDir(qmlModulePath).filePath(filePath)).toString();
    }
};


