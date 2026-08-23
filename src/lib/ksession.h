/*
 *    This file is part of Konsole QML plugin,
 *    which is a terminal emulator from KDE.
 * 
 *    Copyright 2013      by Dmitry Zagnoyko <hiroshidi@gmail.com>
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *    02110-1301  USA.
 */

#pragma once

#include <QObject>

#include "Session.h"

using namespace Konsole;

/**
 * @brief Adapts a Konsole session for use by the Terminal QML control.
 *
 * It configures and starts the shell process, attaches terminal displays,
 * exposes process and working-directory state, and provides history, search,
 * input, and signal operations. This class is exposed to the QML engine as `Session`.
 *
 * @note This class is not part of any public API and it is only part of the Terminal QML control implementation
 */
class KSession : public QObject
{
    Q_OBJECT

    /**
     *  Allows to select the preferred key binding, by default there is one pre-defined.
     */
    Q_PROPERTY(QString kbScheme READ getKeyBindings WRITE setKeyBindings NOTIFY changedKeyBindings)

    /**
     * Set the initial working directory from a local path
     */
    Q_PROPERTY(QString initialWorkingDirectory READ getInitialWorkingDirectory WRITE setInitialWorkingDirectory NOTIFY initialWorkingDirectoryChanged)

    /**
     * The session title
     */
    Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged)

    /**
     * Allows to change the default shell program, by default bash is used
     */
    Q_PROPERTY(QString shellProgram READ shellProgram WRITE setShellProgram NOTIFY shellProgramChanged)

    /**
     * Allows to set the arguments to the default shell program
     */
    Q_PROPERTY(QStringList shellProgramArgs READ args WRITE setArgs NOTIFY argsChanged)

    /**
     * The commands history
     */
    Q_PROPERTY(QString history READ getHistory)

    /**
     * Whether the session has an active process running
     */
    Q_PROPERTY(bool hasActiveProcess READ hasActiveProcess NOTIFY hasActiveProcessChanged)

    /**
     * The name of the current process running
     */
    Q_PROPERTY(QString foregroundProcessName READ foregroundProcessName NOTIFY foregroundProcessNameChanged)

    /**
     * The current directory of the session
     */
    Q_PROPERTY(QString currentDir READ currentDir NOTIFY currentDirChanged)

    /**
     * Allows to set the amount of lines to store in the history
     */
    Q_PROPERTY(int historySize READ historySize WRITE setHistorySize NOTIFY historySizeChanged)

    /**
     * Whether to monitor when the session has gone silent
     */
    Q_PROPERTY(bool monitorSilence READ monitorSilence WRITE setMonitorSilence NOTIFY monitorSilenceChanged)
    
public:
    KSession(QObject *parent = nullptr);
    ~KSession();
    
public:
    /** Attaches  display to this terminal session. */
    void addView(TerminalDisplay *display);

    /** Detaches  display from this terminal session. */
    void removeView(TerminalDisplay *display);
    
    int getRandomSeed();
    QString getKeyBindings();

    /** Sets the environment passed to the shell process. */
    void setEnvironment(const QStringList & environment);
    
    /** Sets the directory in which the shell process starts. */
    void setInitialWorkingDirectory(const QString & dir);
    QString getInitialWorkingDirectory();
    
    /** Sets the codec used to encode and decode terminal text. The default is UTF-8. */
    void setTextCodec(QTextCodec * codec);
    
    /** Sets the scrollback line limit. A negative value enables file-backed history. */
    void setHistorySize(int lines); //infinite if lines < 0
    int historySize() const;
    
    QString getHistory() const;
    
    /**
     * @brief Sets whether flow control is enabled
     * @param enabled
     */
    void setFlowControlEnabled(bool enabled);
    
    /**
     * @brief Returns whether flow control is enabled
     * @return
     */
    bool flowControlEnabled(void);
    
    /**
     * Sets whether the flow control warning box should be shown
     * when the flow control stop key (Ctrl+S) is pressed.
     */
    //void setFlowControlWarningEnabled(bool enabled);
    
    /**
     * @brief Get all available keyboard bindings
     * @return
     */
    static QStringList availableKeyBindings();
    
    /**
     * @brief Return current key bindings
     * @return
     */
    QString keyBindings();
    
    QString getTitle();
    
    /**
     * @brief Returns \c true if the session has an active subprocess running in it
     * spawned from the initial shell.
     */
    bool hasActiveProcess() const;
    
    /**
     * @brief Returns the name of the terminal's foreground process.
     */
    QString foregroundProcessName();
    
    /**
     * @brief Returns the current working directory of the process.
     */
    QString currentDir();
    
    /** Enables or disables silence monitoring for the terminal process. */
    void setMonitorSilence(bool value);

    /** Returns whether silence monitoring is enabled. */
    bool monitorSilence() const;

Q_SIGNALS:
    /** Emitted when the terminal process starts. */
    void started();

    /** Emitted when the terminal process finishes. */
    void finished();

    /** Emitted when the availability of copyable selected text changes. */
    void copyAvailable(bool);
    
    /** Emitted when the terminal display gains focus. */
    void termGetFocus();

    /** Emitted when the terminal display loses focus. */
    void termLostFocus();
    
    /** Forwards a key event received by the terminal display. */
    void termKeyPressed(QKeyEvent *, bool);
    
    /** Emitted when the active keyboard-translation scheme changes. */
    void changedKeyBindings(QString kb);
    
    /** Emitted when the session title changes. */
    void titleChanged();
    
    /** Emitted when the scrollback limit changes. */
    void historySizeChanged();
    
    /** Emitted when the configured initial working directory changes. */
    void initialWorkingDirectoryChanged();
    
    /** Reports the terminal coordinates of a search match. */
    void matchFound(int startColumn, int startLine, int endColumn, int endLine);

    /** Emitted when a terminal-history search finds no match. */
    void noMatchFound();

    /** Emitted when the presence of an active subprocess changes. */
    void hasActiveProcessChanged();

    /** Emitted when the foreground process name changes. */
    void foregroundProcessNameChanged();
    
    /** Reports whether the monitored process has become silent. */
    void processHasSilent(bool value);

    /** Emitted when the terminal requests an audible or visual bell. */
    void bellRequest(QString message);

    /** Emitted when silence monitoring is enabled or disabled. */
    void monitorSilenceChanged();

    /** Emitted when the terminal working directory changes. */
    void currentDirChanged();
    
    /** Emitted when the configured shell executable changes. */
    void shellProgramChanged();

    /** Emitted when the shell argument list changes. */
    void argsChanged();

public Q_SLOTS:
    /**
     * @brief Set named key binding for the session
     */
    void setKeyBindings(const QString & kb);

    /** Sets the session title to @p name. */
    void setTitle(QString name);
    
    /** Starts the configured shell program unless it is already running. */
    void startShellProgram();
    
    /** Sends @p signal to the running process and returns whether it was sent. */
    bool sendSignal(int signal);
    
    /** Sets the shell executable. It initially follows the SHELL environment variable. */
    void setShellProgram(const QString & progname);

    /** Returns the configured shell executable. */
    QString shellProgram() const;

    /** Sets the arguments passed to the shell executable. */
    void setArgs(const QStringList &args);

    /** Returns the arguments passed to the shell executable. */
    QStringList args() const;

    /** Returns the process identifier of the shell process. */
    int getShellPID();

    /** Requests that an idle foreground shell change to @p dir. */
    void changeDir(const QString & dir);
    
    /** Sends @p text to the terminal as process input. */
    void sendText(QString text);

    /**
     * @brief Emulate a key press
     * @param rep
     * @param key
     * @param mod
     */
    void sendKey(int rep, int key, int mod) const;
    
    /** Clears the visible terminal screen. */
    void clearScreen();
    
    /**
     * @brief Search history
     * @param regexp
     * @param startLine
     * @param startColumn
     * @param forwards
     */
    void search(const QString &regexp, int startLine = 0, int startColumn = 0, bool forwards = true );
    
    void selectionChanged(bool textSelected);
    
    int getForegroundProcessError() const;
    
protected Q_SLOTS:
    void sessionFinished();
    
private Q_SLOTS:
     std::unique_ptr<Session> createSession(QString name);   
    
private:
    QString _initialWorkingDirectory;
    std::unique_ptr<Session> m_session;
    QString m_processName;
};
