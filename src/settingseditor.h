#ifndef TVSEG_UI_SETTINGSEDITOR_H
#define TVSEG_UI_SETTINGSEDITOR_H

#include <tvseg/settings/settings.h>

#include <QWidget>
#include <QMap>
#include <QtVariantPropertyManager>
#include <QtTreePropertyBrowser>


namespace tvseg_ui {


class SettingsEditor : public QWidget
{
    Q_OBJECT

public: // types
    typedef tvseg::settings::Settings Settings;
    typedef tvseg::settings::EntryPtr EntryPtr;
    typedef tvseg::settings::Settings::entry_map_type entry_map_type;

public:
    explicit SettingsEditor(QWidget *parent = 0);

    QVector<QtProperty*> addEntries(const Settings &s) {
        // TODO: handle children and/or maybe parse prefix and names for subgrouping
        return addEntries(s.entries(), s.prefix());
    }

    /// Returns vector of added property objects (without the toplevel item); Can be used to identify properties in the updateEntry signal
    QVector<QtProperty*> addEntries(const entry_map_type &s, const std::string &groupName = "Settings");

    void clear();

    void updateFromSettings();

signals:
    void valueChanged(QtProperty *property, QVariant value);

public slots:
    void updateEntry(QtProperty *property, QVariant value);

private:
    void updateFromEntry(QtVariantProperty* property, const EntryPtr &entry);

private:
    QtVariantPropertyManager *variantManager_;
    QtVariantEditorFactory *variantFactory_;
    QtTreePropertyBrowser *variantEditor_;
    QMap<QtVariantProperty*, EntryPtr> callbackEntryMap_;

};

} // namespace tvseg_ui

#endif // TVSEG_UI_SETTINGSEDITOR_H
