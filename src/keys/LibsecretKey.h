#ifndef KEEPASSX_LIBSECRETKEY_H
#define KEEPASSX_LIBSECRETKEY_H

#include "keys/Key.h"

#include <libsecret/secret.h>

class LibsecretKey : public Key
{
public:
    LibsecretKey();

    QByteArray rawKey() const;
    LibsecretKey* clone() const;

    bool load(QString *errorMsg);

private:
    QByteArray theKey;

    static const SecretSchema* getSchema();
    bool generate(QString *errorMsg);
};

#endif // KEEPASSX_LIBSECRETKEY_H
