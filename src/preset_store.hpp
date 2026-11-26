#pragma once

#include "generator.hpp"

#include <QJsonObject>
#include <QString>

namespace ventu::presets {

inline constexpr auto SchemaId = "ventusample.preset.v1";

QJsonObject toJson(const Parameters& parameters, const QString& name, bool includeSeed = false);
bool fromJson(const QJsonObject& document, const Parameters& current,
              Parameters& restored, QString& name, QString& error);
bool saveToFile(const QString& path, const QString& name, const Parameters& parameters,
                bool includeSeed, QString& error);
bool loadFromFile(const QString& path, const Parameters& current,
                  Parameters& restored, QString& name, QString& error);

} // namespace ventu::presets
