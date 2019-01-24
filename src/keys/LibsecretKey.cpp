#include "keys/LibsecretKey.h"
#include "crypto/Random.h"

#include <QDebug>

LibsecretKey::LibsecretKey()
{
}

QByteArray LibsecretKey::rawKey() const {
    return theKey;
}

LibsecretKey* LibsecretKey::clone() const {
    return new LibsecretKey(*this);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
inline const SecretSchema* LibsecretKey::getSchema() {
    static const SecretSchema _schema = {
        "org.keepassx.keystorage",
        SECRET_SCHEMA_NONE,
        {
            { nullptr }
        }
    };
    return &_schema;
}
#pragma GCC diagnostic pop

bool LibsecretKey::load(QString *errorMsg) {
    GError *error = nullptr;
    gchar *key = secret_password_lookup_sync(getSchema(), nullptr, &error,
                                             nullptr);

    if (error != nullptr) {
        if (errorMsg) *errorMsg = error->message;
        g_error_free(error);
        return false;
    } else if (key == nullptr) {
        return generate(errorMsg);
    } else {
        // decode the key
        theKey = QByteArray::fromBase64(key);
        secret_password_free(key);
        return true;
    }
}

bool LibsecretKey::generate(QString *errorMsg) {
    theKey = randomGen()->randomArray(32);
    GError *error = nullptr;
    secret_password_store_sync(getSchema(), SECRET_COLLECTION_DEFAULT,
                               "Key for KeePassX database", theKey.toBase64(),
                               nullptr, &error,
                               nullptr
                               );
    if (error != nullptr) {
        if (errorMsg) *errorMsg = error->message;
        g_error_free(error);
        return false;
    } else {
        return true;
    }
}
