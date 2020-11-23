#include "profile.h"
#include <QByteArray>
#include <stdexcept>
#include <iostream>

namespace QSS {

struct ProfilePrivate {
    bool httpProxy = false;
    bool debug = false;
    std::string pluginExec;
    std::string pluginOpts;
};

Profile::Profile() :
    d_private(new ProfilePrivate()),
    d_localAddress("127.0.0.1"),
    d_serverPort(0),
    d_localPort(0),
    d_timeout(600)
{
}

Profile::Profile(const Profile &b) :
    d_private(new ProfilePrivate(*b.d_private)),
    d_name(b.d_name),
    d_method(b.d_method),
    d_password(b.d_password),
    d_serverAddress(b.d_serverAddress),
    d_localAddress(b.d_localAddress),
    d_serverPort(b.d_serverPort),
    d_localPort(b.d_localPort),
    d_timeout(b.d_timeout)
{
}

Profile::Profile(Profile &&b)
    : d_private(std::move(b.d_private))
    , d_name(std::move(b.d_name))
    , d_method(std::move(b.d_method))
    , d_password(std::move(b.d_password))
    , d_serverAddress(std::move(b.d_serverAddress))
    , d_localAddress(std::move(b.d_localAddress))
    , d_serverPort(std::move(b.d_serverPort))
    , d_localPort(std::move(b.d_localPort))
    , d_timeout(std::move(b.d_timeout))
{
}

Profile::~Profile()
{
}

const std::string& Profile::name() const
{
    return d_name;
}

const std::string& Profile::method() const
{
    return d_method;
}

const std::string& Profile::password() const
{
    return d_password;
}

const std::string& Profile::serverAddress() const
{
    return d_serverAddress;
}

const std::string& Profile::localAddress() const
{
    return d_localAddress;
}

const std::string& Profile::pluginExec() const
{
    return d_private->pluginExec;
}

const std::string& Profile::pluginOpts() const
{
    return d_private->pluginOpts;
}

uint16_t Profile::serverPort() const
{
    return d_serverPort;
}

uint16_t Profile::localPort() const
{
    return d_localPort;
}

int Profile::timeout() const
{
    return d_timeout;
}

bool Profile::debug() const
{
    return d_private->debug;
}

bool Profile::httpProxy() const
{
    return d_private->httpProxy;
}

bool Profile::isValid() const
{
    return !method().empty() && !password().empty() && !serverAddress().empty();
}

bool Profile::hasPlugin() const
{
    return !d_private->pluginExec.empty();
}

void Profile::setName(const std::string& name)
{
    d_name = name;
}

void Profile::setMethod(const std::string& method)
{
    d_method = method;
}

void Profile::setPassword(const std::string& password)
{
    d_password = password;
}

void Profile::setServerAddress(const std::string& server)
{
    d_serverAddress = server;
}

void Profile::setLocalAddress(const std::string& local)
{
    d_localAddress = local;
}

void Profile::setServerPort(uint16_t p)
{
    d_serverPort = p;
}

void Profile::setLocalPort(uint16_t p)
{
    d_localPort = p;
}

void Profile::setTimeout(int t)
{
    d_timeout = t;
}

void Profile::setHttpProxy(bool e)
{
    d_private->httpProxy = e;
}

void Profile::enableDebug()
{
    d_private->debug = true;
}

void Profile::disableDebug()
{
    d_private->debug = false;
}

void Profile::setPlugin(std::string exec, std::string opts)
{
    d_private->pluginExec = std::move(exec);
    d_private->pluginOpts = std::move(opts);
}

void Profile::unsetPlugin()
{
    d_private->pluginExec.clear();
    d_private->pluginOpts.clear();
}

Profile Profile::fromUri(const std::string& ssUri)
{
    if (ssUri.length() < 5) {
        std::cerr << "xxxxxxxxxxxxxxxxxxxx1" << std::endl;
        throw std::invalid_argument("SS URI is too short");
    }

    Profile result;
    //remove the prefix "ss://" from uri
    std::string uri(ssUri.data() + 5, ssUri.length() - 5);
    std::cout<< uri << std::endl;
    size_t hashPos = uri.find_last_of('#');
    if (hashPos != std::string::npos) {
        // Get the name/remark
        result.setName(uri.substr(hashPos + 1));
        uri.erase(hashPos);
    }
    size_t pluginPos = uri.find_first_of('/');
    if (pluginPos != std::string::npos) {
        // TODO: support plugins. For now, just ignore them
        uri.erase(pluginPos);
    }
    size_t atPos = uri.find_first_of('@');
    if (atPos == std::string::npos) {
        // Old URI scheme
        std::string decoded(QByteArray::fromBase64(QByteArray(uri.data(), uri.length()), QByteArray::Base64Option::OmitTrailingEquals).data());
        std::cout << decoded << std::endl;
        size_t colonPos = decoded.find_first_of(':');
        if (colonPos == std::string::npos) {
            std::cerr << "xxxxxxxxxxxxxxxxxxxxxxxx2" << std::endl;
            throw std::invalid_argument("Can't find the colon separator between method and password");
        }
        std::string method = decoded.substr(0, colonPos);
        result.setMethod(method);
        decoded.erase(0, colonPos + 1);
        atPos = decoded.find_last_of('@');
        if (atPos == std::string::npos) {
            std::cerr << "xxxxxxxxxxxxxxxxxxxxxxxxx3" << std::endl;
            throw std::invalid_argument("Can't find the at separator between password and hostname");
        }
        result.setPassword(decoded.substr(0, atPos));
        decoded.erase(0, atPos + 1);
        colonPos = decoded.find_last_of(':');
        if (colonPos == std::string::npos) {
            std::cerr << "xxxxxxxxxxxxxxxxxxxxxxxxx4" << std::endl;
            throw std::invalid_argument("Can't find the colon separator between hostname and port");
        }
        result.setServerAddress(decoded.substr(0, colonPos));
        result.setServerPort(std::stoi(decoded.substr(colonPos + 1)));
    } else {
        // SIP002 URI scheme
        std::string userInfo(QByteArray::fromBase64(QByteArray(uri.data(), atPos), QByteArray::Base64Option::Base64UrlEncoding).data());
        size_t userInfoSp = userInfo.find_first_of(':');
        if (userInfoSp == std::string::npos) {
            std::cerr << "xxxxxxxxxxxxxxxxxxxxxxxxxx5" << std::endl;
            throw std::invalid_argument("Can't find the colon separator between method and password");
        }
        std::string method = userInfo.substr(0, userInfoSp);
        result.setMethod(method);
        result.setPassword(userInfo.substr(userInfoSp + 1));

        uri.erase(0, atPos + 1);
        size_t hostSpPos = uri.find_last_of(':');
        if (hostSpPos == std::string::npos) {
            std::cerr << "xxxxxxxxxxxxxxxxxxxxxxxx6" << std::endl;
            throw std::invalid_argument("Can't find the colon separator between hostname and port");
        }
        result.setServerAddress(uri.substr(0, hostSpPos));
        result.setServerPort(std::stoi(uri.substr(hostSpPos + 1)));
    }

    return result;
}

std::string Profile::toUri() const
{
    std::string ssUri = method() + ":" + password() + "@" + serverAddress() + ":" + std::to_string(serverPort());
    QByteArray uri = QByteArray(ssUri.data()).toBase64(QByteArray::Base64Option::OmitTrailingEquals);
    uri.prepend("ss://");
    uri.append("#");
    uri.append(d_name.data(), d_name.length());
    return std::string(uri.data(), uri.length());
}

std::string Profile::toUriSip002() const
{
    std::string plainUserInfo = method() + ":" + password();
    std::string userinfo(QByteArray(plainUserInfo.data()).toBase64(QByteArray::Base64Option::Base64UrlEncoding).data());
    return "ss://" + userinfo + "@" + serverAddress() + ":" + std::to_string(serverPort()) + "#" + name();
}

}  // namespace QSS
