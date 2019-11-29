#include "desktop-menu-plugin-manager.h"

#include "style-plugin-iface.h"

#include <QDir>
#include <QPluginLoader>
#include <QtConcurrent>
#include <QApplication>
#include <QProxyStyle>

#include <QDebug>

using namespace Peony;

static DesktopMenuPluginManager *global_instance = nullptr;
static bool m_is_loading = false;

DesktopMenuPluginManager::DesktopMenuPluginManager(QObject *parent) : QObject(parent)
{
    m_is_loading = true;
    loadAsync();
}

DesktopMenuPluginManager::~DesktopMenuPluginManager()
{
    for (auto plugin : m_map) {
        delete plugin;
    }
    m_map.clear();
}

void DesktopMenuPluginManager::loadAsync()
{
    qDebug()<<"test start";
    QDir pluginsDir("/usr/lib/peony-qt-extensions");
    pluginsDir.setFilter(QDir::Files);
    Q_FOREACH(QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        if (!plugin)
            continue;

        StylePluginIface *splugin = dynamic_cast<StylePluginIface*>(plugin);
        if (splugin) {
            QApplication::setStyle(splugin->getStyle());
            break;
        }
    }

    QtConcurrent::run([=](){
        qDebug()<<pluginsDir.entryList().count();
        Q_FOREACH(QString fileName, pluginsDir.entryList(QDir::Files)) {
            qDebug()<<fileName;
            QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
            qDebug()<<pluginLoader.fileName();
            qDebug()<<pluginLoader.metaData();
            qDebug()<<pluginLoader.load();
            QObject *plugin = pluginLoader.instance();
            if (!plugin)
                continue;

            MenuPluginInterface *piface = dynamic_cast<MenuPluginInterface*>(plugin);
            if (!piface)
                continue;
            qDebug()<<"ok";
            if (!m_map.value(piface->name()))
                m_map.insert(piface->name(), piface);
            m_is_loaded = true;
        }
    });
}

DesktopMenuPluginManager *DesktopMenuPluginManager::getInstance()
{
    if (!global_instance) {
        if (!m_is_loading) {
            m_is_loading = true;
            global_instance = new DesktopMenuPluginManager;
            global_instance->loadAsync();
        }
    }
    return global_instance;
}

const QStringList DesktopMenuPluginManager::getPluginIds()
{
    return m_map.keys();
}

MenuPluginInterface *DesktopMenuPluginManager::getPlugin(const QString &pluginId)
{
    return m_map.value(pluginId);
}

QList<MenuPluginInterface*> DesktopMenuPluginManager::getPlugins()
{
    return m_map.values();
}
