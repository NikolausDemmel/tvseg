#include "settingseditor.h"


#include "qtpropertymanager.h"
#include "qtvariantproperty.h"
#include "qttreepropertybrowser.h"

#include "tvseg/util/arrayio.h"

#include <boost/foreach.hpp>

#include <QVBoxLayout>

namespace tvseg_ui {


namespace {


using namespace tvseg::settings;


typedef QPair<QString, QVariant> AttributePair;
typedef QList<AttributePair> AttributeList;


QPair<QVariant, AttributeList> anyToVariant(const boost::any &value)
{
    QVariant var;
    AttributeList attributes;

    // bool
    if (var.isNull()) {
        const bool* p = boost::any_cast<bool>(&value);
        if (p) {
            var = *p;
        }
    }

    // std::string
    if (var.isNull()) {
        const std::string* p = boost::any_cast<std::string>(&value);
        if (p) {
            var = QString::fromStdString(*p);
        }
    }

    // double
    if (var.isNull()) {
        const double* p = boost::any_cast<double>(&value);
        if (p) {
            var = *p;
        }
    }

    // int
    if (var.isNull()) {
        const int* p = boost::any_cast<int>(&value);
        if (p) {
            var = *p;
        }
    }

    // uint -> int
    if (var.isNull()) {
        const unsigned int* p = boost::any_cast<unsigned int>(&value);
        if (p) {
            var = static_cast<int>(*p);
            attributes.append(AttributePair("minimum", 0));
        }
    }

    // float -> double
    if (var.isNull()) {
        const float* p = boost::any_cast<float>(&value);
        if (p) {
            var = static_cast<double>(*p);
            attributes.append(AttributePair("decimals", 8));
        }
    }

    // std::vector<float> -> string
    if (var.isNull()) {
        const float_array_t *p = boost::any_cast<float_array_t>(&value);
        if (p) {
            std::ostringstream ss;
            ss << *p;
            var = QString::fromStdString(ss.str());
        }
    }

    // std::vector<uint> -> string
    if (var.isNull()) {
        const uint_array_t *p = boost::any_cast<uint_array_t>(&value);
        if (p) {
            std::ostringstream ss;
            ss << *p;
            var = QString::fromStdString(ss.str());
        }
    }

    return QPair<QVariant, AttributeList>(var, attributes);
}


bool variantAssignAny(boost::any &value, const QVariant &var)
{
    // bool
    if (boost::any_cast<bool>(&value)) {
        if (var.type() == QVariant::Bool) {
            value = var.toBool();
            return true;
        } else {
            LWARNING << "variantAssignAny: variant type '" << var.typeName() << "' cannot be assigned to bool value";
            return false;
        }
    }

    // std::string
    if (boost::any_cast<std::string>(&value)) {
        if (var.type() == QVariant::String) {
            value = var.toString().toStdString();
            return true;
        } else {
            LWARNING << "variantAssignAny: variant type '" << var.typeName() << "' cannot be assigned to std::string value";
            return false;
        }
    }

    // double
    if (boost::any_cast<double>(&value)) {
        if (var.type() == QVariant::Double) {
            value = var.toDouble();
            return true;
        } else {
            LWARNING << "variantAssignAny: variant type '" << var.typeName() << "' cannot be assigned to double value";
            return false;
        }
    }

    // int
    if (boost::any_cast<int>(&value)) {
        if (var.type() == QVariant::Int) {
            value = var.toInt();
            return true;
        } else {
            LWARNING << "variantAssignAny: variant type '" << var.typeName() << "' cannot be assigned to int value";
            return false;
        }
    }

    // uint -> int
    if (boost::any_cast<unsigned int>(&value)) {
        if (var.type() == QVariant::Int) {
            if (var.toInt() < 0) {
                LWARNING << "variantAssignAny: variant value " << var.toInt() << " for unsigned int is negative. Using 0.";
            }
            value = static_cast<unsigned int>(std::max(0, var.toInt()));
            return true;
        } else {
            LWARNING << "variantAssignAny: variant type '" << var.typeName() << "' cannot be assigned to unsigned int value";
            return false;
        }
    }

    // float -> double
    if (boost::any_cast<float>(&value)) {
        if (var.type() == QVariant::Double) {
            value = static_cast<float>(var.toDouble());
            return true;
        } else {
            LWARNING << "variantAssignAny: variant type '" << var.typeName() << "' cannot be assigned to float value";
            return false;
        }
    }

    // std::vector<float> -> string
    if (boost::any_cast<float_array_t>(&value)) {
        if (var.type() == QVariant::String) {
            float_array_t tmp;
            std::istringstream ss(var.toString().toStdString());
            ss >> tmp;
            value = tmp;
            return true;
        } else {
            LWARNING << "variantAssignAny: variant type '" << var.typeName() << "' cannot be assigned to float_array_t value";
            return false;
        }
    }

    // std::vector<uint> -> string
    if (boost::any_cast<uint_array_t>(&value)) {
        if (var.type() == QVariant::String) {
            uint_array_t tmp;
            std::istringstream ss(var.toString().toStdString());
            ss >> tmp;
            value = tmp;
            return true;
        } else {
            LWARNING << "variantAssignAny: variant type '" << var.typeName() << "' cannot be assigned to uint_array_t value";
            return false;
        }
    }


    // conversion not implemented
    return false;
}


QtVariantProperty* createProperty(QtVariantPropertyManager* variantManager, SettingsEditor::EntryPtr entry, QString name)
{
    QPair<QVariant, AttributeList> pair = anyToVariant(entry->value());
    QVariant &var = pair.first;
    AttributeList &attributes = pair.second;

    // not implemented
    if (var.isNull()) {
        LWARNING << "Could not add property '" << name << "' of type '" << entry->value().type().name() << "' to editor. Conversion not implemented.";
        return NULL;
    }

    QtVariantProperty* property = variantManager->addProperty(var.type(), name);

    if (!property) {
        LWARNING << "Could not add property '" << name << "' of type '" << entry->value().type().name() << "' to editor. Not supported by PropertyBrowser.";
        return NULL;
    }

    property->setValue(var);

    foreach(const AttributePair &p, attributes) {
        variantManager->setAttribute(property, p.first, p.second);
    }

    return property;
}


} // namespace



SettingsEditor::SettingsEditor(QWidget *parent) :
    QWidget(parent),
    variantManager_(new QtVariantPropertyManager(this)),
    variantFactory_(new QtVariantEditorFactory(this)),
    variantEditor_(new QtTreePropertyBrowser(this))
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(variantEditor_);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);

    variantEditor_->setFactoryForManager(variantManager_, variantFactory_);
    variantEditor_->setPropertiesWithoutValueMarked(true);
    variantEditor_->setRootIsDecorated(false);
    variantEditor_->setResizeMode(QtTreePropertyBrowser::Interactive);

    connect(variantManager_, SIGNAL(valueChanged(QtProperty*,QVariant)), this, SLOT(updateEntry(QtProperty*,QVariant)));
    connect(variantManager_, SIGNAL(valueChanged(QtProperty*,QVariant)), this, SIGNAL(valueChanged(QtProperty*,QVariant)));
}

QVector<QtProperty*> SettingsEditor::addEntries(const entry_map_type &entries, const std::string &groupName)
{
    QtProperty *topItem = variantManager_->addProperty(QtVariantPropertyManager::groupTypeId(), QString::fromStdString(groupName));

    QVector<QtProperty*> addedProperties;

    BOOST_FOREACH(entry_map_type::value_type p, entries) {
        std::string name = p.first;
        EntryPtr entry = p.second;
        QtVariantProperty* item = createProperty(variantManager_, entry, QString::fromStdString(name));
        if (item) {
            topItem->addSubProperty(item);
            callbackEntryMap_[item] = entry;
            addedProperties.push_back(item);
        }
    }

    variantEditor_->addProperty(topItem);

    return addedProperties;
}

void SettingsEditor::clear()
{
    variantManager_->clear();
    variantEditor_->clear();
    callbackEntryMap_.clear();
}

void SettingsEditor::updateFromSettings()
{
    foreach(QtVariantProperty* key, callbackEntryMap_.keys()) {
        updateFromEntry(key, callbackEntryMap_[key]);
    }
}

void SettingsEditor::updateFromEntry(QtVariantProperty *property, const EntryPtr &entry)
{
    QPair<QVariant, AttributeList> pair = anyToVariant(entry->value());
    QVariant &var = pair.first;
    AttributeList &attributes = pair.second;

    // not implemented
    if (var.isNull()) {
        LWARNING << "Could not update property '" << property->propertyName() << "' of type '" << entry->value().type().name() << "'. Conversion not implemented.";
        return;
    }

    property->setValue(var);

    foreach(const AttributePair &p, attributes) {
        variantManager_->setAttribute(property, p.first, p.second);
    }
}

void SettingsEditor::updateEntry(QtProperty *property, QVariant value)
{
    QtVariantProperty *vp = static_cast<QtVariantProperty*>(property);

    QMap<QtVariantProperty*, EntryPtr>::const_iterator iter = callbackEntryMap_.find(vp);

    if (iter != callbackEntryMap_.end()) {
        boost::any oldValue = iter.value()->value();
        if (!variantAssignAny(oldValue, value)) {
            LWARNING << "Cannot convert variant to any for setting'" << property->propertyName() << "'";
        } else {
            if (!iter.value()->setValue(oldValue)) {
                LWARNING << "Cannot update entry from setting '" << property->propertyName() << "'";
            }
        }
    }
}

} // namespace tvseg_ui
