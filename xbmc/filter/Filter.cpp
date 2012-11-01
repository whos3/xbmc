/*
 *      Copyright (C) 2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "Filter.h"
#include "guilib/LocalizeStrings.h"
#include "utils/StringUtils.h"
#include "utils/Variant.h"

using namespace std;

typedef struct {
  std::string id;
  Field field;
  FilterFieldType type;
  bool browseable;
  int label;
} FilterField;

static const FilterField fields[] = {
  { "none",              FieldNone,                    FilterFieldTypeNone,     false,   231 },
  { "filename",          FieldFilename,                FilterFieldTypeString,   false,   561 },
  { "path",              FieldPath,                    FilterFieldTypeString,   true,    573 },
  { "album",             FieldAlbum,                   FilterFieldTypeString,   true,    558 },
  { "albumartist",       FieldAlbumArtist,             FilterFieldTypeString,   true,    566 },
  { "artist",            FieldArtist,                  FilterFieldTypeString,   true,    557 },
  { "tracknumber",       FieldTrackNumber,             FilterFieldTypeInteger,  false,   554 },
  { "comment",           FieldComment,                 FilterFieldTypeString,   false,   569 },
  { "review",            FieldReview,                  FilterFieldTypeString,   false,   183 },
  { "themes",            FieldThemes,                  FilterFieldTypeString,   false, 21895 },
  { "moods",             FieldMoods,                   FilterFieldTypeString,   false,   175 },
  { "styles",            FieldStyles,                  FilterFieldTypeString,   false,   176 },
  { "type",              FieldAlbumType,               FilterFieldTypeString,   false,   564 }, // TODO: browseable
  { "label",             FieldMusicLabel,              FilterFieldTypeString,   false, 21899 },
  { "title",             FieldTitle,                   FilterFieldTypeString,   false,   556 },
  { "sorttitle",         FieldSortTitle,               FilterFieldTypeString,   false,   556 },
  { "year",              FieldYear,                    FilterFieldTypeString,   true,    562 },
  { "time",              FieldTime,                    FilterFieldTypeTime,     false,   180 },
  { "playcount",         FieldPlaycount,               FilterFieldTypeInteger,  false,   567 },
  { "lastplayed",        FieldLastPlayed,              FilterFieldTypeDate,     false,   568 },
  { "inprogress",        FieldInProgress,              FilterFieldTypeBoolean,  false,   575 },
  { "rating",            FieldRating,                  FilterFieldTypeNumber,   false,   563 },
  { "votes",             FieldVotes,                   FilterFieldTypeString,   false,   205 },
  { "top250",            FieldTop250,                  FilterFieldTypeInteger,  false, 13409 },
  { "mpaarating",        FieldMPAA,                    FilterFieldTypeString,   false, 20074 },
  { "dateadded",         FieldDateAdded,               FilterFieldTypeDate,     false,   570 },
  { "genre",             FieldGenre,                   FilterFieldTypeString,   true,    515 },
  { "plot",              FieldPlot,                    FilterFieldTypeString,   false,   207 },
  { "plotoutline",       FieldPlotOutline,             FilterFieldTypeString,   false,   203 },
  { "tagline",           FieldTagline,                 FilterFieldTypeString,   false,   202 },
  { "set",               FieldSet,                     FilterFieldTypeString,   true,  20457 },
  { "director",          FieldDirector,                FilterFieldTypeString,   true,  20339 },
  { "actor",             FieldActor,                   FilterFieldTypeString,   true,  20337 },
  { "writers",           FieldWriter,                  FilterFieldTypeString,   true,  20417 },
  { "airdate",           FieldAirDate,                 FilterFieldTypeDate,     false, 20416 },
  { "hastrailer",        FieldTrailer,                 FilterFieldTypeBoolean,  false, 20423 },
  { "studio",            FieldStudio,                  FilterFieldTypeString,   true,    572 },
  { "country",           FieldCountry,                 FilterFieldTypeString,   true,    574 },
  { "tvshow",            FieldTvShowTitle,             FilterFieldTypeString,   true,  20364 },
  { "status",            FieldTvShowStatus,            FilterFieldTypeString,   false,   126 },
  { "season",            FieldSeason,                  FilterFieldTypeInteger,  false, 20373 },
  { "episode",           FieldEpisodeNumber,           FilterFieldTypeInteger,  false, 20359 },
  { "numepisodes",       FieldNumberOfEpisodes,        FilterFieldTypeInteger,  false, 20360 },
  { "numwatched",        FieldNumberOfWatchedEpisodes, FilterFieldTypeInteger,  false, 21441 },
  { "videoresolution",   FieldVideoResolution,         FilterFieldTypeInteger,  false, 21443 }, // TODO: type
  { "videocodec",        FieldVideoCodec,              FilterFieldTypeString,   false, 21445 },
  { "videoaspect",       FieldVideoAspectRatio,        FilterFieldTypeNumber,   false, 21374 },
  { "audiochannels",     FieldAudioChannels,           FilterFieldTypeInteger,  false, 21444 }, // TODO: type
  { "audiocodec",        FieldAudioCodec,              FilterFieldTypeString,   false, 21446 },
  { "audiolanguage",     FieldAudioLanguage,           FilterFieldTypeString,   false, 21447 },
  { "subtitlelanguage",  FieldSubtitleLanguage,        FilterFieldTypeString,   false, 21448 },
  { "random",            FieldRandom,                  FilterFieldTypeString,   false,   590 },
  { "playlist",          FieldPlaylist,                FilterFieldTypePlaylist, false,   559 },
  { "tag",               FieldTag,                     FilterFieldTypeString,   true,  20459 },
  { "instruments",       FieldInstruments,             FilterFieldTypeString,   false, 21892 },
  { "biography",         FieldBiography,               FilterFieldTypeString,   false, 21887 },
  { "born",              FieldBorn,                    FilterFieldTypeString,   false, 21893 },
  { "bandformed",        FieldBandFormed,              FilterFieldTypeString,   false, 21894 },
  { "disbanded",         FieldDisbanded,               FilterFieldTypeString,   false, 21896 },
  { "died",              FieldDied,                    FilterFieldTypeString,   false, 21897 }
};

#define NUM_FIELDS sizeof(fields) / sizeof(FilterField)

typedef struct {
  std::string id;
  CFilterOperator op;
  int label;
} FilterOperator;

static const FilterOperator operators[] = {
  { "",               CFilterOperator(),                                    0 },
  { "contains",       CFilterOperator(FilterOperationContains),         21400 },
  { "doesnotcontain", CFilterOperator(FilterOperationContains, true),   21401 },
  { "is",             CFilterOperator(FilterOperationEquals),           21402 },
  { "isnot",          CFilterOperator(FilterOperationEquals, true),     21403 },
  { "startswith",     CFilterOperator(FilterOperationStartsWith),       21404 },
  { "endswith",       CFilterOperator(FilterOperationEndsWith),         21405 },
  { "greaterthan",    CFilterOperator(FilterOperationGreaterThan),      21406 },
  { "lessthan",       CFilterOperator(FilterOperationLessThan),         21407 },
  { "after",          CFilterOperator(FilterOperationAfter),            21408 },
  { "before",         CFilterOperator(FilterOperationBefore),           21409 },
  { "inthelast",      CFilterOperator(FilterOperationInTheLast),        21410 },
  { "notinthelast",   CFilterOperator(FilterOperationInTheLast, true),  21411 },
  { "true",           CFilterOperator(FilterOperationTrue),             20122 },
  { "false",          CFilterOperator(FilterOperationTrue, true),       20424 },
  { "between",        CFilterOperator(FilterOperationBetween),          21456 }
};

#define NUM_OPERATORS sizeof(operators) / sizeof(FilterOperator)

CFilter::CFilter()
{
  Reset();
}

void CFilter::Reset()
{
  m_ruleCombination.m_combinations.clear();
  m_ruleCombination.m_rules.clear();
  m_ruleCombination.SetType(CFilterRuleCombination::CombinationAnd);
  m_type = MediaTypeSong;
}

bool CFilter::Load(const CVariant &obj)
{
  if (!obj.isObject())
    return false;
  
  // load the playlist type
  if (obj.isMember("type") && obj["type"].isString())
    m_type = DatabaseUtils::MediaTypeFromString(obj["type"].asString());

  if (obj.isMember("rules"))
    m_ruleCombination.Load(obj["rules"]);

  return true;
}

bool CFilter::Save(CVariant &obj) const
{
  if (obj.type() == CVariant::VariantTypeConstNull)
    return false;
  
  obj.clear();

  // add "type"
  obj["type"] = DatabaseUtils::MediaTypeToString(m_type);
  
  // add "rules"
  CVariant rulesObj = CVariant(CVariant::VariantTypeObject);
  if (m_ruleCombination.Save(rulesObj))
    obj["rules"] = rulesObj;

  return true;
}

void CFilter::GetAvailableFields(std::vector<std::string> &fieldList)
{
  for (unsigned int index = 0; index < NUM_FIELDS; index++)
    fieldList.push_back(fields[index].id);
}

const std::string& CFilter::TranslateField(Field field)
{
  for (unsigned int i = 0; i < NUM_FIELDS; i++)
  {
    if (field == fields[i].field)
      return fields[i].id;
  }
  return fields[0].id;
}

Field CFilter::TranslateField(const std::string &field)
{
  for (unsigned int i = 0; i < NUM_FIELDS; i++)
  {
    if (StringUtils::EqualsNoCase(field, fields[i].id))
      return fields[i].field;
  }
  return fields[0].field;
}

std::string CFilter::GetLocalizedField(Field field)
{
  for (unsigned int i = 0; i < NUM_FIELDS; i++)
  {
    if (field == fields[i].field)
      return g_localizeStrings.Get(fields[i].label);
  }
  return "";
}

void CFilter::GetAvailableOperators(std::vector<std::string> &operatorList)
{
  for (unsigned int index = 1; index < NUM_OPERATORS; index++)
    operatorList.push_back(operators[index].id);
}

const std::string& CFilter::TranslateOperator(const CFilterOperator &op)
{
  for (unsigned int i = 1; i < NUM_OPERATORS; i++)
  {
    if (op == operators[i].op)
      return operators[i].id;
  }
  return operators[0].id;
}

const CFilterOperator& CFilter::TranslateOperator(const std::string &op)
{
  for (unsigned int i = 1; i < NUM_OPERATORS; i++)
  {
    if (StringUtils::EqualsNoCase(op, operators[i].id))
      return operators[i].op;
  }
  return operators[0].op;
}

std::string CFilter::GetLocalizedOperator(const CFilterOperator &op)
{
  for (unsigned int i = 1; i < NUM_OPERATORS; i++)
  {
    if (op == operators[i].op)
      return g_localizeStrings.Get(operators[i].label);
  }
  return "";
}
