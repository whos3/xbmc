#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
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

#include "system.h"

#ifdef HAS_WEB_SERVER_HTTPS
const char* TLS_PRIVATE_KEY =
  "-----BEGIN RSA PRIVATE KEY-----\r\n"
  "MIICWwIBAAKBgQCjbYr6wm0L1FAEnEwXqNMyQjUMTKsRtoI53GG3ccXqlDx7WQ/L\r\n"
  "2P58e6IC/Xa3arsNjrHe+BMgC2GS1OGZj5cAm6DvvtyESFhnJMVlFHRrOc8ZJFYQ\r\n"
  "In07R1UBjQ1IzxBZSbVUCfnoGEHBuH7KbeMfgjzMgNRMWiex9uWm3T1x1wIDAQAB\r\n"
  "AoGAQ5aqQ8rlDl9gOIfrcF2B+ucmyU9r8IdqlENwpk2HbNyhpuHOhp/Xn+QBvPrW\r\n"
  "fkTF13WMWop+XmJWmEMyii+4Yv7Hg/obB1I3vmOsZYosEvulVK+hnjDu1VHklHbQ\r\n"
  "LIeBDVug7JOBBmfk6RyfyU53gKK3qhYl0ytxx4gjb95hPwECQQDO5pcepbI6DUk5\r\n"
  "+8L+dlTeNU8DieDSwD4CqKldue1oC/9jfijyve9WMWyPPI4ieHsX+CqBqOlTKdT/\r\n"
  "g70WRQQVAkEAyjXwCMy4PXPDSZcSTqXb87Og0zZxoTiMOPF/hR5u0xd/aKZQ+v9H\r\n"
  "Qc5Ic9QpuR+LFM5Vs3VZMUJLClH4RIq9OwJAQfjS3OnWr54G6F83qO3QV95FqIox\r\n"
  "TuIPo8dl4cXkRlX1BCN5KRWZGDHAPNgBNkqWbWkrHJbgdy0ShrrZ8xzklQJANvB8\r\n"
  "rQdiL2AaeQ4QrUmm/wmY8oniSZlhj78dlH92QOgrd7VoXymopJp1WPMV+vQ1B4wg\r\n"
  "AMgrWZTXapBm0ciYLQJAOpVVa/+SS8DWwUit4x0an0w8O0KyBvNwaBkJxKJqkVyh\r\n"
  "E04GNvTybBQnYwmKJ3EEUYdBpU4WVBqKovss2tdDSA==\r\n"
  "-----END RSA PRIVATE KEY-----";

const char* TLS_CERTIFICATE =
  "-----BEGIN CERTIFICATE-----\r\n"
  "MIIDXDCCAsWgAwIBAgIJAJL796AMDLjsMA0GCSqGSIb3DQEBBQUAMH0xCzAJBgNV\r\n"
  "BAYTAkNIMRMwEQYDVQQIEwpTdC4gR2FsbGVuMRIwEAYDVQQHEwlPYmVydXp3aWwx\r\n"
  "GDAWBgNVBAoTD1hCTUMgRm91bmRhdGlvbjENMAsGA1UEAxMEWEJNQzEcMBoGCSqG\r\n"
  "SIb3DQEJARYNaW5mb0B4Ym1jLm9yZzAeFw0xMTA0MTAyMDM0MjBaFw0xMjA0MDky\r\n"
  "MDM0MjBaMH0xCzAJBgNVBAYTAkNIMRMwEQYDVQQIEwpTdC4gR2FsbGVuMRIwEAYD\r\n"
  "VQQHEwlPYmVydXp3aWwxGDAWBgNVBAoTD1hCTUMgRm91bmRhdGlvbjENMAsGA1UE\r\n"
  "AxMEWEJNQzEcMBoGCSqGSIb3DQEJARYNaW5mb0B4Ym1jLm9yZzCBnzANBgkqhkiG\r\n"
  "9w0BAQEFAAOBjQAwgYkCgYEAo22K+sJtC9RQBJxMF6jTMkI1DEyrEbaCOdxht3HF\r\n"
  "6pQ8e1kPy9j+fHuiAv12t2q7DY6x3vgTIAthktThmY+XAJug777chEhYZyTFZRR0\r\n"
  "aznPGSRWECJ9O0dVAY0NSM8QWUm1VAn56BhBwbh+ym3jH4I8zIDUTFonsfblpt09\r\n"
  "cdcCAwEAAaOB4zCB4DAdBgNVHQ4EFgQUVaaZyga85Br9mbq39v/GMqTUjVkwgbAG\r\n"
  "A1UdIwSBqDCBpYAUVaaZyga85Br9mbq39v/GMqTUjVmhgYGkfzB9MQswCQYDVQQG\r\n"
  "EwJDSDETMBEGA1UECBMKU3QuIEdhbGxlbjESMBAGA1UEBxMJT2JlcnV6d2lsMRgw\r\n"
  "FgYDVQQKEw9YQk1DIEZvdW5kYXRpb24xDTALBgNVBAMTBFhCTUMxHDAaBgkqhkiG\r\n"
  "9w0BCQEWDWluZm9AeGJtYy5vcmeCCQCS+/egDAy47DAMBgNVHRMEBTADAQH/MA0G\r\n"
  "CSqGSIb3DQEBBQUAA4GBAAyh8AmPuJMKBHPkS9+cxC2tJFZCIRCKYWUwz+yasWme\r\n"
  "uePO5du88KLX8R+us14ktCyKYfcGOxNHrWhXP567sah1d19vXECw9jIUA9Uty1v+\r\n"
  "L4jekEaIe1CN5a4LVxmPzbR1W+VlGO5s2KAqGtWGZAkHRWmY7y7+O7bfq8l2/dfa\r\n"
  "-----END CERTIFICATE-----";
#endif
