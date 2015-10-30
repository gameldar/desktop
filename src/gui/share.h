/*
 * Copyright (C) by Roeland Jago Douma <rullzer@owncloud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#ifndef SHARE_H
#define SHARE_H

#include "accountfwd.h"

#include <QObject>
#include <QDate>
#include <QString>
#include <QList>
#include <QSharedPointer>
#include <QUrl>

namespace OCC {

class Share : public QObject {
    Q_OBJECT

public:

    /**
     * Possible share types
     */
    enum ShareType {
        TypeUser   = 0,
        TypeGroup  = 1,
        TypeLink   = 3,
        TypeRemote = 6,
    };
    Q_DECLARE_FLAGS(ShareTypes, ShareType)

    /**
     * Possible permissions
     */
    enum Permission {
        PermissionRead   =  1,
        PermissionUpdate =  2,
        PermissionCreate =  4,
        PermissionDelete =  8,
        PermissionShare  = 16
    };
    Q_DECLARE_FLAGS(Permissions, Permission)

    /*
     * Constructor for shares
     */
    explicit Share(AccountPtr account,
                   const QString& id,
                   const QString& path,
                   ShareType shareType,
                   Permissions permissions);

    /*
     * Get the id
     */
    QString getId() const;

    /*
     * Get the shareType
     */
    ShareType getShareType() const;

    /*
     * Get permissions
     */
    Permissions getPermissions() const;

    /*
     * Set the permissions of a share
     *
     * On success the permissionsSet signal is emitted
     * In case of a server error the serverError signal is emitted.
     */
    void setPermissions(int permissions);

    /**
     * Deletes a share
     *
     * On success the shareDeleted signal is emitted
     * In case of a server error the serverError signal is emitted.
     */
    void deleteShare();

signals:
    void permissionsSet();
    void shareDeleted();
    void serverError(int code, const QString &message);

protected:
    AccountPtr _account;
    QString _id;
    QString _path;
    ShareType _shareType;
    Permissions _permissions;

protected slots:
    void slotOcsError(int statusCode, const QString &message);

private slots:
    void slotDeleted();

};

/**
 * A Link share is just like a regular share but then slightly different.
 * There are several methods in the API that either work differently for
 * link shares or are only available to link shares.
 */
class LinkShare : public Share {
    Q_OBJECT
public:
 
    explicit LinkShare(AccountPtr account,
                       const QString& id,
                       const QString& path,
                       Permissions permissions,
                       bool passwordSet,
                       const QUrl& url,
                       const QDate& expireDate);

    /*
     * Get the share link
     */
    QUrl getLink() const;

    /*
     * Get the publicUpload status of this share
     */
    bool getPublicUpload();

    /*
     * Set a share to be public upload
     * This function can only be called on link shares
     *
     * On success the publicUploadSet signal is emitted
     * In case of a server error the serverError signal is emitted.
     */
    void setPublicUpload(bool publicUpload);
    
    /*
     * Set the password
     *
     * On success the passwordSet signal is emitted
     * In case of a server error the serverError signal is emitted.
     */
    void setPassword(const QString& password);

    /*
     * Is the password set?
     */
    bool isPasswordSet() const;

    /*
     * Get the expiration date
     */
    QDate getExpireDate() const;

    /*
     * Set the expiration date
     *
     * On success the expireDateSet signal is emitted
     * In case of a server error the serverError signal is emitted.
     */
    void setExpireDate(const QDate& expireDate);

signals:
    void expireDateSet();
    void publicUploadSet();
    void passwordSet();

private slots:
    void slotPasswordSet(const QVariantMap&, const QVariant &value);
    void slotPublicUploadSet(const QVariantMap&, const QVariant &value);
    void slotExpireDateSet(const QVariantMap&, const QVariant &value);

private:
    bool _passwordSet;
    QDate _expireDate;
    QUrl _url;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Share::Permissions)

/**
 * The share manager allows for creating, retrieving and deletion
 * of shares. It abstracts away from the OCS Share API, all the usages
 * shares should talk to this manager and not use OCS Share Job directly
 */
class ShareManager : public QObject {
    Q_OBJECT
public:
    explicit ShareManager(AccountPtr _account, QObject *parent = NULL);

    /**
     * Tell the manager to create a link share
     *
     * @param path The path of the linkshare relative to the user folder on the server
     * @param password The password of the share
     *
     * On success the signal linkShareCreated is emitted
     * For older server the linkShareRequiresPassword signal is emitted when it seems appropiate
     * In case of a server error the serverError signal is emitted
     */
    void createLinkShare(const QString& path,
                         const QString& password="");

    /**
     * Fetch all the shares for path
     *
     * @param path The path to get the shares for relative to the users folder on the server
     *
     * On success the sharesFetched signal is emitted
     * In case of a server error the serverError signal is emitted
     */
    void fetchShares(const QString& path);

signals:
    void linkShareCreated(const QSharedPointer<LinkShare> &share);
    void linkShareRequiresPassword();
    void sharesFetched(const QList<QSharedPointer<Share>> &shares);
    void serverError(int code, const QString &message);

private slots:
    void slotSharesFetched(const QVariantMap &reply);
    void slotLinkShareCreated(const QVariantMap &reply);
    void slotOcsError(int statusCode, const QString &message);

private:
    QSharedPointer<LinkShare> parseLinkShare(const QVariantMap &data);

    AccountPtr _account;
};


}

#endif // SHARE_H
