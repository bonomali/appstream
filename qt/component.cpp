/*
 * Copyright (C) 2016 Matthias Klumpp <matthias@tenstral.net>
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the license, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "appstream.h"
#include "component.h"

#include "screenshot.h"
#include "release.h"
#include <QSharedData>
#include <QStringList>
#include <QUrl>
#include <QMap>
#include <QMultiHash>

using namespace AppStream;

typedef QHash<Component::Kind, QString> KindMap;
Q_GLOBAL_STATIC_WITH_ARGS(KindMap, kindMap, ( {
    { Component::KindGeneric, QLatin1String("generic") },
    { Component::KindDesktop, QLatin1String("desktop-application") },
    { Component::KindConsoleApp, QLatin1String("console-application") },
    { Component::KindWebApp, QLatin1String("web-application") },
    { Component::KindAddon, QLatin1String("addon") },
    { Component::KindFont, QLatin1String("font") },
    { Component::KindCodec, QLatin1String("codec") },
    { Component::KindInputmethod, QLatin1String("inputmethod") },
    { Component::KindFirmware, QLatin1String("firmware") },
    { Component::KindDriver, QLatin1String("driver") },
    { Component::KindLocalization, QLatin1String("localization") },
    { Component::KindUnknown, QLatin1String("unknown") }
    }
));

QString Component::kindToString(Component::Kind kind) {
    return kindMap->value(kind);
}

Component::Kind Component::stringToKind(const QString& kindString) {
    if(kindString ==  QLatin1String("")) {
        return KindGeneric;
    }
    if(kindString ==  QLatin1String("generic"))
        return KindGeneric;

    if (kindString == QLatin1String("desktop-application"))
        return KindDesktop;

    if (kindString == QLatin1String("console-application"))
        return KindConsoleApp;

    if (kindString == QLatin1String("web-application"))
        return KindWebApp;

    if (kindString == QLatin1String("addon"))
        return KindAddon;

    if (kindString == QLatin1String("font"))
        return KindFont;

    if (kindString == QLatin1String("codec"))
        return KindCodec;

    if (kindString==QLatin1String("inputmethod"))
        return KindInputmethod;

    if (kindString == QLatin1String("firmware"))
        return KindFirmware;

    if (kindString == QLatin1String("driver"))
        return KindDriver;

    if (kindString == QLatin1String("localization"))
        return KindLocalization;

    return KindUnknown;

}

Component::UrlKind Component::stringToUrlKind(const QString& urlKindString) {
    if (urlKindString == QLatin1String("homepage")) {
        return UrlKindHomepage;
    }
    if (urlKindString == QLatin1String("bugtracker")) {
        return UrlKindBugtracker;
    }
    if (urlKindString == QLatin1String("faq")) {
        return UrlKindFaq;
    }
    if (urlKindString == QLatin1String("help")) {
        return UrlKindHelp;
    }
    if (urlKindString == QLatin1String("donation")) {
        return UrlKindDonation;
    }
    return UrlKindUnknown;
}

typedef QHash<Component::UrlKind, QString> UrlKindMap;
Q_GLOBAL_STATIC_WITH_ARGS(UrlKindMap, urlKindMap, ({
        { Component::UrlKindBugtracker, QLatin1String("bugtracker") },
        { Component::UrlKindDonation, QLatin1String("donation") },
        { Component::UrlKindFaq, QLatin1String("faq") },
        { Component::UrlKindHelp, QLatin1String("help") },
        { Component::UrlKindHomepage, QLatin1String("homepage") },
        { Component::UrlKindUnknown, QLatin1String("unknown") },
    }));

QString Component::urlKindToString(Component::UrlKind kind) {
    return urlKindMap->value(kind);
}

QString Component::bundleKindToString(Component::BundleKind kind) {
    switch (kind) {
        case Component::BundleKindLimba:
            return QStringLiteral("limba");
        case Component::BundleKindFlatpak:
            return QStringLiteral("flatpak");
        default:
            return QString();
    }
}

Component::BundleKind Component::stringToBundleKind(const QString& bundleKindString) {
    if (bundleKindString == QLatin1String("limba")) {
        return BundleKindLimba;
    }
    if (bundleKindString == QLatin1String("flatpak")) {
        return BundleKindFlatpak;
    }
    return BundleKindUnknown;
}

QString value(const gchar *cstr)
{
    return QString::fromUtf8(cstr);
}

QStringList value(gchar **strv)
{
    QStringList res;
    if (strv == NULL)
        return res;
    for (uint i = 0; strv[i] != NULL; i++) {
        res.append (value(strv[i]));
    }
    return res;
}

QStringList value(GPtrArray *array)
{
    QStringList res;
    res.reserve(array->len);
    for (uint i = 0; i < array->len; i++) {
        auto strval = (const gchar*) g_ptr_array_index (array, i);
        res.append (value(strval));
    }
    return res;
}

Component::Component(const Component& other)
    : m_cpt(other.m_cpt)
{
    g_object_ref(m_cpt);
}

Component::Component()
{
    m_cpt = as_component_new();
}

Component::Component(_AsComponent *cpt)
    : m_cpt(cpt)
{
    g_object_ref(m_cpt);
}

Component::~Component()
{
    g_object_unref(m_cpt);
}

Component::Kind Component::kind() const
{
    return static_cast<Component::Kind>(as_component_get_kind (m_cpt));
}

void Component::setKind(Component::Kind kind)
{
    as_component_set_kind(m_cpt, static_cast<AsComponentKind>(kind));
}

QString Component::id() const
{
    return value(as_component_get_id(m_cpt));
}

void Component::setId(const QString& id)
{
    as_component_set_id(m_cpt, qPrintable(id));
}

QString Component::dataId() const
{
    return value(as_component_get_data_id(m_cpt));
}

void Component::setDataId(const QString& cdid)
{
    as_component_set_data_id(m_cpt, qPrintable(cdid));
}

QString Component::desktopId() const
{
    return value(as_component_get_desktop_id(m_cpt));
}

QStringList Component::packageNames() const
{
    return value(as_component_get_pkgnames(m_cpt));
}

QString Component::name() const
{
    return value(as_component_get_name(m_cpt));
}

void Component::setName(const QString& name, const QString& lang)
{
    as_component_set_name(m_cpt, qPrintable(name), qPrintable(lang));
}

QString Component::summary() const
{
    return value(as_component_get_summary(m_cpt));
}

void Component::setSummary(const QString& summary, const QString& lang)
{
    as_component_set_summary(m_cpt, qPrintable(summary), qPrintable(lang));
}

QString Component::description() const
{
    return value(as_component_get_description(m_cpt));
}

void Component::setDescription(const QString& description, const QString& lang)
{
    as_component_set_description(m_cpt, qPrintable(description), qPrintable(lang));
}

QString Component::projectLicense() const
{
    return value(as_component_get_project_license(m_cpt));
}

void Component::setProjectLicense(const QString& license)
{
    as_component_set_project_license(m_cpt, qPrintable(license));
}

QString Component::projectGroup() const
{
    return value(as_component_get_project_group(m_cpt));
}

void Component::setProjectGroup(const QString& group)
{
    as_component_set_project_group(m_cpt, qPrintable(group));
}

QString Component::developerName() const
{
    return value(as_component_get_developer_name(m_cpt));
}

void Component::setDeveloperName(const QString& developerName, const QString& lang)
{
    as_component_set_developer_name(m_cpt, qPrintable(developerName), qPrintable(lang));
}

QStringList Component::compulsoryForDesktops() const
{
    return value(as_component_get_compulsory_for_desktops(m_cpt));
}

bool Component::isCompulsoryForDesktop(const QString& desktop) const
{
    return as_component_is_compulsory_for_desktop(m_cpt, qPrintable(desktop));
}

QStringList Component::categories() const
{
    return value(as_component_get_categories(m_cpt));
}

bool Component::hasCategory(const QString& category) const
{
    return as_component_has_category(m_cpt, qPrintable(category));
}

QStringList Component::extends() const
{
    return value(as_component_get_extends(m_cpt));
}

QList<AppStream::Component> Component::addons() const
{
    QList<AppStream::Component> res;

    auto addons = as_component_get_addons (m_cpt);
    res.reserve(addons->len);
    for (uint i = 0; i < addons->len; i++) {
        auto cpt = AS_COMPONENT (g_ptr_array_index (addons, i));
        res.append(Component(cpt));
    }
    return res;
}

QUrl Component::url(Component::UrlKind kind) const
{
    return QUrl(as_component_get_url(m_cpt, static_cast<AsUrlKind>(kind)));
}
