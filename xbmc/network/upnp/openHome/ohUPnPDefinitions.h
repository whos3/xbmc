#pragma once
/*
 *      Copyright (C) 2014 Team XBMC
 *      http://xbmc.org
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

#define UPNP_DOMAIN_NAME                      "upnp.org"

#define UPNP_DIDL_LITE_NAMESPACE_URI          "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/"
#define UPNP_DIDL_UPNP_NAMESPACE_URI          "urn:schemas-upnp-org:metadata-1-0/upnp/"
#define UPNP_DIDL_DC_NAMESPACE_URI            "http://purl.org/dc/elements/1.1/"
#define UPNP_DIDL_DLNA_NAMESPACE_URI          "urn:schemas-dlna-org:metadata-1-0/"

#define UPNP_DIDL_UPNP_NAMESPACE              "upnp"
#define UPNP_DIDL_DC_NAMESPACE                "dc"
#define UPNP_DIDL_DLNA_NAMESPACE              "dlna"

#define UPNP_SERVICE_TYPE_IDENTIFIER          "urn:schemas-upnp-org:service:%s:%d"
#define UPNP_SERVICE_TYPE_AVTRANSPORT         "AVTransport"
#define UPNP_SERVICE_TYPE_CONNECTIONMANAGER   "ConnectionManager"
#define UPNP_SERVICE_TYPE_CONTENTDIRECTORY    "ContentDirectory"
#define UPNP_SERVICE_TYPE_RENDERINGCONTROL    "RenderingControl"
#define UPNP_SERVICE_TYPE_SCHEDULEDRECORDING  "ScheduledRecording"

#define UPNP_DEVICE_TYPE_IDENTIFIER           "urn:schemas-upnp-org:device:%s:%d"
#define UPNP_DEVICE_TYPE_MEDIARENDERER        "MediaRenderer"
#define UPNP_DEVICE_TYPE_MEDIASERVER          "MediaServer"

#define UPNP_DEVICE_ATTRIBUTE_FRIENDLY_NAME   "Upnp.FriendlyName"
#define UPNP_DEVICE_ATTRIBUTE_LOCATION        "Upnp.Location"
#define UPNP_DEVICE_ATTRIBUTE_DEVICE_XML      "Upnp.DeviceXml"

#define UPNP_ERROR_INVALID_ACTION               401
#define UPNP_ERROR_INVALID_ARGS                 402
#define UPNP_ERROR_ACTION_FAILED                501
#define UPNP_ERROR_INVALID_ARGUMENT_VALUE       600
#define UPNP_ERROR_ARGUMENT_VALUE_OUT_OF_RANGE  601
#define UPNP_ERROR_ACTION_NOT_IMPLEMENTED       602
#define UPNP_ERROR_OUT_OF_MEMORY                603
#define UPNP_ERROR_HUMAN_INTERVENTION_REQUIRED  604
#define UPNP_ERROR_STRING_ARGUMENT_TOO_LONG     605

#define UPNP_ERROR_CM_INCOMPATIBLE_PROTOCOL         701
#define UPNP_ERROR_CM_INCOMPATIBLE_DIRECTION        702
#define UPNP_ERROR_CM_INSUFFICIENT_NET_RESOURCES    703
#define UPNP_ERROR_CM_LOCAL_RESTRICTIONS            704
#define UPNP_ERROR_CM_ACCESS_DENIED                 705
#define UPNP_ERROR_CM_INVALID_CONNECTION_REFERENCE  706
#define UPNP_ERROR_CM_NOT_IN_NETWORK                707

#define UPNP_ERROR_AVT_INVALID_TRANSITION       701
#define UPNP_ERROR_AVT_NO_CONTENTS              702
#define UPNP_ERROR_AVT_READ_ERROR               703
#define UPNP_ERROR_AVT_UNSUPPORTED_PLAY_FORMAT  704
#define UPNP_ERROR_AVT_TRANSPORT_LOCKED         705
#define UPNP_ERROR_AVT_WRITE_ERROR              706
#define UPNP_ERROR_AVT_PROTECTED_MEDIA          707
#define UPNP_ERROR_AVT_UNSUPPORTED_REC_FORMAT   708
#define UPNP_ERROR_AVT_FULL_MEDIA               709
#define UPNP_ERROR_AVT_UNSUPPORTED_SEEK_MODE    710
#define UPNP_ERROR_AVT_ILLEGAL_SEEK_TARGET      711
#define UPNP_ERROR_AVT_UNSUPPORTED_PLAY_MODE    712
#define UPNP_ERROR_AVT_UNSUPPORTED_REC_QUALITY  713
#define UPNP_ERROR_AVT_ILLEGAL_MIME             714
#define UPNP_ERROR_AVT_CONTENT_BUSY             715
#define UPNP_ERROR_AVT_RESOURCE_NOT_FOUND       716
#define UPNP_ERROR_AVT_UNSUPPORTED_PLAY_SPEED   717
#define UPNP_ERROR_AVT_INVALID_INSTANCE_ID      718

#define UPNP_ERROR_RC_INVALID_PRESET_NAME       701
#define UPNP_ERROR_RC_INVALID_INSTANCE_ID       702
#define UPNP_ERROR_RC_INVALID_CHANNEL           703

#define UPNP_ERROR_CD_NO_SUCH_OBJECT                      701
#define UPNP_ERROR_CD_INVALID_CURRENTTAGVALUE             702
#define UPNP_ERROR_CD_INVALID_NEWTAGVALUE                 703
#define UPNP_ERROR_CD_REQUIRED_TAG_DELETE                 704
#define UPNP_ERROR_CD_READONLY_TAG_UPDATE                 705
#define UPNP_ERROR_CD_PARAMETER_NUM_MISMATCH              706
#define UPNP_ERROR_CD_BAD_SEARCH_CRITERIA                 708
#define UPNP_ERROR_CD_BAD_SORT_CRITERIA                   709
#define UPNP_ERROR_CD_NO_SUCH_CONTAINER                   710
#define UPNP_ERROR_CD_RESTRICTED_OBJECT                   711
#define UPNP_ERROR_CD_BAD_METADATA                        712
#define UPNP_ERROR_CD_RESTRICTED_PARENT_OBJECT            713
#define UPNP_ERROR_CD_NO_SUCH_SOURCE_RESOURCE             714
#define UPNP_ERROR_CD_SOURCE_RESOURCE_ACCESS_DENIED       715
#define UPNP_ERROR_CD_TRANSFER_BUSY                       716
#define UPNP_ERROR_CD_NO_SUCH_FILE_TRANSFER               717
#define UPNP_ERROR_CD_NO_SUCH_DESTINATION_RESOURCE        718
#define UPNP_ERROR_CD_DESTINATION_RESOURCE_ACCESS_DENIED  719
#define UPNP_ERROR_CD_REQUEST_FAILED                      720
