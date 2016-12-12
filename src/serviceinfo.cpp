/*
 * Copyright (c) 2016 Intel Corporation.  All rights reserved.
 * See the bottom of this file for the license terms.
 *
 * Modified for Joule demo project - 
 */

#include "serviceinfo.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
ServiceInfo::ServiceInfo()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
ServiceInfo::ServiceInfo(QLowEnergyService *service):
    m_service(service)
{
    m_service->setParent(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
QLowEnergyService *ServiceInfo::service() const
{
    return m_service;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString ServiceInfo::getName() const
{
    if (!m_service)
        return QString();

    return m_service->serviceName();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString ServiceInfo::getType() const
{
    if (!m_service)
        return QString();

    QString result;
    if (m_service->type() & QLowEnergyService::PrimaryService)
        result += QStringLiteral("primary");
    else
        result += QStringLiteral("secondary");

    if (m_service->type() & QLowEnergyService::IncludedService)
        result += QStringLiteral(" included");

    result.prepend('<').append('>');

    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString ServiceInfo::getUuid() const
{
    if (!m_service)
        return QString();

    const QBluetoothUuid uuid = m_service->serviceUuid();
    bool success = false;
    quint16 result16 = uuid.toUInt16(&success);
    if (success)
        return QStringLiteral("0x") + QString::number(result16, 16);

    quint32 result32 = uuid.toUInt32(&success);
    if (success)
        return QStringLiteral("0x") + QString::number(result32, 16);

    return uuid.toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
   Copyright (c) 2016 Intel Corporation.  All rights reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited. All rights reserved.
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
