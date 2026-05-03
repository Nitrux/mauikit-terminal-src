/*
    SPDX-FileCopyrightText: 2007-2008 Robert Knight <robertknight@gmail.countm>

    SPDX-License-Identifier: GPL-2.0-or-later

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

// Own
#include "ProcessInfo.h"

// Unix
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <unistd.h>

// Qt
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QFlags>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtNetwork/QHostInfo>

// KDE
// #include <KConfigGroup>
// #include <KSharedConfig>
#include <QDebug>

using namespace Konsole;

ProcessInfo::ProcessInfo(int aPid, bool enableEnvironmentRead)
    : _fields(ARGUMENTS | ENVIRONMENT) // arguments and environments
    // are currently always valid,
    // they just return an empty
    // vector / map respectively
    // if no arguments
    // or environment bindings
    // have been explicitly set
    , _enableEnvironmentRead(enableEnvironmentRead)
    , _pid(aPid)
    , _parentPid(0)
    , _foregroundPid(0)
    , _userId(0)
    , _lastError(NoError)
    , _userName(QString())
    , _userHomeDir(QString())
{
}

ProcessInfo::Error ProcessInfo::error() const
{
    return _lastError;
}
void ProcessInfo::setError(Error error)
{
    _lastError = error;
}

void ProcessInfo::update()
{
    readProcessInfo(_pid, _enableEnvironmentRead);
}

QString ProcessInfo::validCurrentDir() const
{
    bool ok = false;

    // read current dir, if an error occurs try the parent as the next
    // best option
    int currentPid = parentPid(&ok);
    QString dir = currentDir(&ok);
    while (!ok && currentPid != 0) {
        auto current = ProcessInfo::newInstance(currentPid);
        current->update();
        currentPid = current->parentPid(&ok);
        dir = current->currentDir(&ok);
    }

    return dir;
}

QString ProcessInfo::format(const QString &input) const
{
    bool ok = false;

    QString output(input);

    // search for and replace known marker
    output.replace(QStringLiteral("%u"), userName());
    output.replace(QStringLiteral("%h"), localHost());
    output.replace(QStringLiteral("%n"), name(&ok));

    QString dir = validCurrentDir();
    if (output.contains(u"%D")) {
        QString homeDir = userHomeDir();
        QString tempDir = dir;
        // Change User's Home Dir w/ ~ only at the beginning
        if (tempDir.startsWith(homeDir)) {
            tempDir.remove(0, homeDir.length());
            tempDir.prepend(u'~');
        }
        output.replace(QStringLiteral("%D"), tempDir);
    }
    output.replace(QStringLiteral("%d"), formatShortDir(dir));

    return output;
}

QSet<QString> ProcessInfo::_commonDirNames;

QSet<QString> ProcessInfo::commonDirNames()
{
    // static bool forTheFirstTime = true;
    //
    // if (forTheFirstTime) {
    //     const KSharedConfigPtr& config = KSharedConfig::openConfig();
    //     const KConfigGroup& configGroup = config->group("ProcessInfo");
    //     _commonDirNames = QSet<QString>::fromList(configGroup.readEntry("CommonDirNames", QStringList()));
    //
    //     forTheFirstTime = false;
    // }

    return _commonDirNames;
}

QString ProcessInfo::formatShortDir(const QString &input) const
{
    QString result;

    const QStringList &parts = input.split(QDir::separator());

    QSet<QString> dirNamesToShorten = commonDirNames();

    QListIterator<QString> iter(parts);
    iter.toBack();

    // go backwards through the list of the path's parts
    // adding abbreviations of common directory names
    // and stopping when we reach a dir name which is not
    // in the commonDirNames set
    while (iter.hasPrevious()) {
        const QString &part = iter.previous();

        if (dirNamesToShorten.contains(part)) {
            result.prepend(QString(QDir::separator()) + part[0]);
        } else {
            result.prepend(part);
            break;
        }
    }

    return result;
}

QVector<QString> ProcessInfo::arguments(bool *ok) const
{
    *ok = _fields.testFlag(ARGUMENTS);

    return _arguments;
}

QMap<QString, QString> ProcessInfo::environment(bool *ok) const
{
    *ok = _fields.testFlag(ENVIRONMENT);

    return _environment;
}

bool ProcessInfo::isValid() const
{
    return _fields.testFlag(PROCESS_ID);
}

int ProcessInfo::pid(bool *ok) const
{
    *ok = _fields.testFlag(PROCESS_ID);

    return _pid;
}

int ProcessInfo::parentPid(bool *ok) const
{
    *ok = _fields.testFlag(PARENT_PID);

    return _parentPid;
}

int ProcessInfo::foregroundPid(bool *ok) const
{
    *ok = _fields.testFlag(FOREGROUND_PID);

    return _foregroundPid;
}

QString ProcessInfo::name(bool *ok) const
{
    *ok = _fields.testFlag(NAME);

    return _name;
}

int ProcessInfo::userId(bool *ok) const
{
    *ok = _fields.testFlag(UID);

    return _userId;
}

QString ProcessInfo::userName() const
{
    return _userName;
}

QString ProcessInfo::userHomeDir() const
{
    return _userHomeDir;
}

QString ProcessInfo::localHost()
{
    return QHostInfo::localHostName();
}

void ProcessInfo::setPid(int aPid)
{
    _pid = aPid;
    _fields |= PROCESS_ID;
}

void ProcessInfo::setUserId(int uid)
{
    _userId = uid;
    _fields |= UID;
}

void ProcessInfo::setUserName(const QString &name)
{
    _userName = name;
    setUserHomeDir();
}

void ProcessInfo::setUserHomeDir()
{
    _userHomeDir = QDir::homePath();
}

void ProcessInfo::setParentPid(int aPid)
{
    _parentPid = aPid;
    _fields |= PARENT_PID;
}
void ProcessInfo::setForegroundPid(int aPid)
{
    _foregroundPid = aPid;
    _fields |= FOREGROUND_PID;
}

QString ProcessInfo::currentDir(bool *ok) const
{
    if (ok)
        *ok = _fields & CURRENT_DIR;

    return _currentDir;
}
void ProcessInfo::setCurrentDir(const QString &dir)
{
    _fields |= CURRENT_DIR;
    _currentDir = dir;
}

void ProcessInfo::setName(const QString &name)
{
    _name = name;
    _fields |= NAME;
}
void ProcessInfo::addArgument(const QString &argument)
{
    _arguments << argument;
}

void ProcessInfo::clearArguments()
{
    _arguments.clear();
}

void ProcessInfo::addEnvironmentBinding(const QString &name, const QString &value)
{
    _environment.insert(name, value);
}

void ProcessInfo::setFileError(QFile::FileError error)
{
    switch (error) {
    case QFile::PermissionsError:
        setError(ProcessInfo::PermissionsError);
        break;
    case QFile::NoError:
        setError(ProcessInfo::NoError);
        break;
    default:
        setError(ProcessInfo::UnknownError);
    }
}

//
// ProcessInfo::newInstance() is way at the bottom so it can see all of the
// implementations of the UnixProcessInfo abstract class.
//

NullProcessInfo::NullProcessInfo(int aPid, bool enableEnvironmentRead)
    : ProcessInfo(aPid, enableEnvironmentRead)
{
}

bool NullProcessInfo::readProcessInfo(int /*pid*/, bool /*enableEnvironmentRead*/)
{
    return false;
}

void NullProcessInfo::readUserName()
{
}

UnixProcessInfo::UnixProcessInfo(int aPid, bool enableEnvironmentRead)
    : ProcessInfo(aPid, enableEnvironmentRead)
{
}

bool UnixProcessInfo::readProcessInfo(int aPid, bool enableEnvironmentRead)
{
    // prevent _arguments from growing longer and longer each time this
    // method is called.
    clearArguments();

    bool ok = readProcInfo(aPid);
    if (ok) {
        ok |= readArguments(aPid);
        ok |= readCurrentDir(aPid);
        if (enableEnvironmentRead) {
            ok |= readEnvironment(aPid);
        }
    }
    return ok;
}

void UnixProcessInfo::readUserName()
{
    bool ok = false;
    const int uid = userId(&ok);
    if (!ok)
        return;

    struct passwd passwdStruct;
    struct passwd *getpwResult;
    char *getpwBuffer;
    long getpwBufferSize;
    int getpwStatus;

    getpwBufferSize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (getpwBufferSize == -1)
        getpwBufferSize = 16384;

    getpwBuffer = new char[getpwBufferSize];
    if (getpwBuffer == nullptr)
        return;
    getpwStatus = getpwuid_r(uid, &passwdStruct, getpwBuffer, getpwBufferSize, &getpwResult);
    if ((getpwStatus == 0) && (getpwResult != nullptr)) {
        setUserName(QString::fromLatin1(passwdStruct.pw_name));
    } else {
        setUserName(QString());
        qWarning() << "getpwuid_r returned error : " << getpwStatus;
    }
    delete[] getpwBuffer;
}

class LinuxProcessInfo : public UnixProcessInfo
{
public:
    LinuxProcessInfo(int aPid, bool env)
        : UnixProcessInfo(aPid, env)
    {
    }

private:
    bool readProcInfo(int aPid) override
    {
        // indicies of various fields within the process status file which
        // contain various information about the process
        const int PARENT_PID_FIELD = 3;
        const int PROCESS_NAME_FIELD = 1;
        const int GROUP_PROCESS_FIELD = 7;

        QString parentPidString;
        QString processNameString;
        QString foregroundPidString;
        QString uidLine;
        QString uidString;
        QStringList uidStrings;

        // For user id read process status file ( /proc/<pid>/status )
        //  Can not use getuid() due to it does not work for 'su'
        QFile statusInfo(QStringLiteral("/proc/%1/status").arg(aPid));
        if (statusInfo.open(QIODevice::ReadOnly)) {
            QTextStream stream(&statusInfo);
            QString statusLine;
            do {
                statusLine = stream.readLine();
                if (statusLine.startsWith(QLatin1String("Uid:")))
                    uidLine = statusLine;
            } while (!statusLine.isNull() && uidLine.isNull());

            uidStrings << uidLine.split(u'\t', Qt::SkipEmptyParts);
            // Must be 5 entries: 'Uid: %d %d %d %d' and
            // uid string must be less than 5 chars (uint)
            if (uidStrings.size() == 5)
                uidString = uidStrings[1];
            if (uidString.size() > 5)
                uidString.clear();

            bool ok = false;
            const int uid = uidString.toInt(&ok);
            if (ok)
                setUserId(uid);
            readUserName();
        } else {
            setFileError(statusInfo.error());
            return false;
        }

        // read process status file ( /proc/<pid/stat )
        //
        // the expected file format is a list of fields separated by spaces, using
        // parenthesies to escape fields such as the process name which may itself contain
        // spaces:
        //
        // FIELD FIELD (FIELD WITH SPACES) FIELD FIELD
        //
        QFile processInfo(QStringLiteral("/proc/%1/stat").arg(aPid));
        if (processInfo.open(QIODevice::ReadOnly)) {
            QTextStream stream(&processInfo);
            const QString &data = stream.readAll();

            int stack = 0;
            int field = 0;
            int pos = 0;

            while (pos < data.size()) {
                QChar c = data[pos];

                if (c == u'(') {
                    stack++;
                } else if (c == u')') {
                    stack--;
                } else if (stack == 0 && c == u' ') {
                    field++;
                } else {
                    switch (field) {
                    case PARENT_PID_FIELD:
                        parentPidString.append(c);
                        break;
                    case PROCESS_NAME_FIELD:
                        processNameString.append(c);
                        break;
                    case GROUP_PROCESS_FIELD:
                        foregroundPidString.append(c);
                        break;
                    }
                }

                pos++;
            }
        } else {
            setFileError(processInfo.error());
            return false;
        }

        // check that data was read successfully
        bool ok = false;
        const int foregroundPid = foregroundPidString.toInt(&ok);
        if (ok)
            setForegroundPid(foregroundPid);

        const int parentPid = parentPidString.toInt(&ok);
        if (ok)
            setParentPid(parentPid);

        if (!processNameString.isEmpty())
            setName(processNameString);

        // update object state
        setPid(aPid);

        return ok;
    }

    bool readArguments(int aPid) override
    {
        // read command-line arguments file found at /proc/<pid>/cmdline
        // the expected format is a list of strings delimited by null characters,
        // and ending in a double null character pair.

        QFile argumentsFile(QStringLiteral("/proc/%1/cmdline").arg(aPid));
        if (argumentsFile.open(QIODevice::ReadOnly)) {
            QTextStream stream(&argumentsFile);
            const QString &data = stream.readAll();

            const QStringList &argList = data.split(QChar(u'\0'));

            for (const QString &entry : argList) {
                if (!entry.isEmpty())
                    addArgument(entry);
            }
        } else {
            setFileError(argumentsFile.error());
        }

        return true;
    }

    bool readCurrentDir(int aPid) override
    {
        char path_buffer[MAXPATHLEN + 1];
        path_buffer[MAXPATHLEN] = 0;
        QByteArray procCwd = QFile::encodeName(QStringLiteral("/proc/%1/cwd").arg(aPid));
        const int length = readlink(procCwd.constData(), path_buffer, MAXPATHLEN);
        if (length == -1) {
            setError(UnknownError);
            return false;
        }

        path_buffer[length] = '\0';
        QString path = QFile::decodeName(path_buffer);

        setCurrentDir(path);
        return true;
    }

    bool readEnvironment(int aPid) override
    {
        // read environment bindings file found at /proc/<pid>/environ
        // the expected format is a list of KEY=VALUE strings delimited by null
        // characters and ending in a double null character pair.

        QFile environmentFile(QStringLiteral("/proc/%1/environ").arg(aPid));
        if (environmentFile.open(QIODevice::ReadOnly)) {
            QTextStream stream(&environmentFile);
            const QString &data = stream.readAll();

            const QStringList &bindingList = data.split(QChar(u'\0'));

            for (const QString &entry : bindingList) {
                QString name;
                QString value;

                const int splitPos = entry.indexOf(u'=');

                if (splitPos != -1) {
                    name = entry.mid(0, splitPos);
                    value = entry.mid(splitPos + 1, -1);

                    addEnvironmentBinding(name, value);
                }
            }
        } else {
            setFileError(environmentFile.error());
        }

        return true;
    }
};

SSHProcessInfo::SSHProcessInfo(const ProcessInfo &process)
    : _process(process)
{
    bool ok = false;

    // check that this is a SSH process
    const QString &name = _process.name(&ok);

    if (!ok || name != u"ssh") {
        if (!ok)
            qWarning() << "Could not read process info";
        else
            qWarning() << "Process is not a SSH process";

        return;
    }

    // read arguments
    const QVector<QString> &args = _process.arguments(&ok);

    // SSH options
    // these are taken from the SSH manual ( accessed via 'man ssh' )

    // options which take no arguments
    static const QStringView noArgumentOptions(u"1246AaCfgKkMNnqsTtVvXxYy");
    // options which take one argument
    static const QStringView singleArgumentOptions(u"bcDeFIiLlmOopRSWw");

    if (ok) {
        // find the username, host and command arguments
        //
        // the username/host is assumed to be the first argument
        // which is not an option
        // ( ie. does not start with a dash '-' character )
        // or an argument to a previous option.
        //
        // the command, if specified, is assumed to be the argument following
        // the username and host
        //
        // note that we skip the argument at index 0 because that is the
        // program name ( expected to be 'ssh' in this case )
        for (int i = 1; i < args.size(); i++) {
            // If this one is an option ...
            // Most options together with its argument will be skipped.
            if (args[i].startsWith(u'-')) {
                const QChar optionChar = (args[i].length() > 1) ? args[i][1] : u'\0';
                // for example: -p2222 or -p 2222 ?
                const bool optionArgumentCombined = args[i].length() > 2;

                if (noArgumentOptions.contains(optionChar)) {
                    continue;
                } else if (singleArgumentOptions.contains(optionChar)) {
                    QString argument;
                    if (optionArgumentCombined) {
                        argument = args[i].mid(2);
                    } else {
                        // Verify correct # arguments are given
                        if ((i + 1) < args.size()) {
                            argument = args[i + 1];
                        }
                        i++;
                    }

                    // support using `-l user` to specify username.
                    if (optionChar == u'l')
                        _user = argument;
                    // support using `-p port` to specify port.
                    else if (optionChar == u'p')
                        _port = argument;

                    continue;
                }
            }

            // check whether the host has been found yet
            // if not, this must be the username/host argument
            if (_host.isEmpty()) {
                // check to see if only a hostname is specified, or whether
                // both a username and host are specified ( in which case they
                // are separated by an '@' character:  username@host )

                const int separatorPosition = args[i].indexOf(u'@');
                if (separatorPosition != -1) {
                    // username and host specified
                    _user = args[i].left(separatorPosition);
                    _host = args[i].mid(separatorPosition + 1);
                } else {
                    // just the host specified
                    _host = args[i];
                }
            } else {
                // host has already been found, this must be the command argument
                _command = args[i];
            }
        }
    } else {
        qWarning() << "Could not read arguments";

        return;
    }
}

QString SSHProcessInfo::userName() const
{
    return _user;
}
QString SSHProcessInfo::host() const
{
    return _host;
}
QString SSHProcessInfo::port() const
{
    return _port;
}
QString SSHProcessInfo::command() const
{
    return _command;
}
QString SSHProcessInfo::format(const QString &input) const
{
    QString output(input);

    // test whether host is an ip address
    // in which case 'short host' and 'full host'
    // markers in the input string are replaced with
    // the full address
    bool isIpAddress = false;

    struct in_addr address;
    if (inet_aton(_host.toLocal8Bit().constData(), &address) != 0)
        isIpAddress = true;
    else
        isIpAddress = false;

    // search for and replace known markers
    output.replace(QStringLiteral("%u"), _user);

    if (isIpAddress)
        output.replace(QStringLiteral("%h"), _host);
    else
        output.replace(QStringLiteral("%h"), _host.left(_host.indexOf(u'.')));

    output.replace(QStringLiteral("%H"), _host);
    output.replace(QStringLiteral("%c"), _command);

    return output;
}

std::unique_ptr<ProcessInfo> ProcessInfo::newInstance(int aPid, bool enableEnvironmentRead)
{
    return std::make_unique<LinuxProcessInfo>(aPid, enableEnvironmentRead);
}
