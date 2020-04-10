#include "common.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <fstream>
#include <pf_log.h>
#include <shlwapi.h>
#pragma comment(lib,"Shlwapi.lib") 
#pragma comment(lib, "pf_log.lib")

using namespace std;

typedef std::map<std::string, std::string> KVStringPair;
unsigned long long m_ullLogInst;
unsigned short m_usLogType;

FtpSession::FtpSession(void)
{
	m_pFtpConn = NULL;
	m_strServerAddr.Empty();
	m_wServerPort = FTP_DEFAULT_PORT;
	m_strUserName.Empty();
	m_strPassWord.Empty();
	m_bInitiativeMode = FALSE;
	m_strLastErrorMsg.Empty();
}

FtpSession::~FtpSession(void)
{
	CloseConnection();
}

// ����FTP��������ַ���˿�
void FtpSession::SetServerParam(CString strServerAddr, INTERNET_PORT wServerPort)
{
	m_strServerAddr = strServerAddr;
	m_wServerPort = wServerPort;
}

// ���õ�¼��������
void FtpSession::SetUserInfo(CString strUserName, CString strPassWord, BOOL bInitiativeMode)
{
	m_strUserName = strUserName;
	m_strPassWord = strPassWord;
	m_bInitiativeMode = bInitiativeMode;
}

// ���ӵ�ָ��FTP������
BOOL FtpSession::ConnectToServer()
{
	try
	{
		CloseConnection();
		// m_bInitiativeMode = TRUE Ϊ����ģʽ
		m_pFtpConn = m_cInetSession.GetFtpConnection(m_strServerAddr,
			m_strUserName, m_strPassWord, m_wServerPort, m_bInitiativeMode);
		if (NULL != m_pFtpConn)
			return TRUE;
	}
	catch (CInternetException *e)
	{
		e->Delete();
	}
	return FALSE;
}

// ��ȡ�����ļ��б�
void FtpSession::GetFileFromDirectory(CString csDirPath, std::vector<FileProperty> &vctFileList)
{
	CString filename = _T("");
	FileProperty sOneFile;
	vctFileList.empty();
	CFileFind find;

	BOOL IsFind = find.FindFile(csDirPath + _T("/*.*"));

	while (IsFind)
	{
		IsFind = find.FindNextFile();
		if (find.IsDots())
		{
			continue;
		}
		else
		{
			// �õ��ļ���
			sOneFile.strFileName = filename = find.GetFileName();
			sOneFile.dwFileType = 0;
			// �ļ���
			if (find.IsDirectory())
				sOneFile.dwFileType |= FILE_ATTRIBUTE_DIRECTORY;
			else {
				sOneFile.dwFileType |= FILE_ATTRIBUTE_NORMAL;	// ��ͨ�ļ�
				// ��ӽ��ļ��б�
				vctFileList.push_back(sOneFile);
			}
		}
	}
}

// �õ�ftp������ָ��Ŀ¼�µ������ļ�
void FtpSession::GetFileList(CString strDirPath, std::vector<FileProperty> &vctFileList)
{
	if (NULL == m_pFtpConn
		&& !ConnectToServer())
		return;

	vctFileList.empty();

	CFtpFileFind cFtpFileFind(m_pFtpConn);
	// ����ָ��Ŀ¼
	BOOL bWorking = cFtpFileFind.FindFile(strDirPath);
	while (bWorking)
	{
		bWorking = cFtpFileFind.FindNextFile();
		// .����..
		if (cFtpFileFind.IsDots())
			continue;
		FileProperty sOneFile;
		// �õ��ļ���
		sOneFile.strFileName = cFtpFileFind.GetFileName();
		sOneFile.dwFileType = 0;
		// �ļ���
		if (cFtpFileFind.IsDirectory())
			sOneFile.dwFileType |= FILE_ATTRIBUTE_DIRECTORY;
		else {
			sOneFile.dwFileType |= FILE_ATTRIBUTE_NORMAL;	// ��ͨ�ļ�
			// ��ӽ��ļ��б�
			vctFileList.push_back(sOneFile);
		}
		
	}
}

// �ϴ�һ���ļ���FTP������
BOOL FtpSession::PutFileToServer(CString strLocalFile, CString strPutPath)
{
	if (NULL == m_pFtpConn
		&& !ConnectToServer())
		return FALSE;

	return m_pFtpConn->PutFile(strLocalFile, strPutPath);
}

// ��FTP����������һ���ļ�
BOOL FtpSession::GetFileFromServer(CString strRemoteFile, CString strLocalfileName, BOOL bFailIfExists)
{
	if (NULL == m_pFtpConn
		&& !ConnectToServer())
		return FALSE;

	return m_pFtpConn->GetFile(strRemoteFile, strLocalfileName, bFailIfExists);
}

// ���ñ��������ļ�·��
BOOL FtpSession::SetLocalFilePath(char* strLocalFilePath)
{
	if (NULL == m_pFtpConn
		&& !ConnectToServer())
		return FALSE;
	
	return m_pFtpConn->SetCurrentDirectory(strLocalFilePath);
}

// ��FTP������ɾ��һ���ļ�
BOOL FtpSession::RemoveFileFromServer(CString strRemoteFile)
{
	if (NULL == m_pFtpConn
		&& !ConnectToServer())
		return FALSE;

	return m_pFtpConn->Remove(strRemoteFile);
}

// ɾ�������ļ�
BOOL FtpSession::RemoveFileFromLocal(CString strRemoteFile)
{
	if (NULL == m_pFtpConn
		&& !ConnectToServer())
		return FALSE;
	
	return DeleteFile(strRemoteFile);
}

// �ر�����
void FtpSession::CloseConnection()
{
	if (NULL != m_pFtpConn)
	{
		m_pFtpConn->Close();
		delete m_pFtpConn;
	}
	m_pFtpConn = NULL;
}

// �������Ƿ����
BOOL FtpSession::IsConnectionActive()
{
	if (NULL == m_pFtpConn)
		return FALSE;
	return TRUE;
}

// ��ȡ���һ��������Ϣ
CString FtpSession::GetLastErrorMessage()
{
	m_strLastErrorMsg.Format(_T("�����룺%ld"), ::GetLastError());
	return m_strLastErrorMsg;
}

// ���������ļ�
bool loadCfg(const char * pFileName_, KVStringPair & kvList_)
{
	bool result = false;
	char szLine[256] = { 0 };
	std::fstream cfgFile;
	cfgFile.open(pFileName_, std::ios::in);
	if (cfgFile.is_open()) {
		while (!cfgFile.eof()) {
			cfgFile.getline(szLine, sizeof(szLine), '\n');
			std::string str = szLine;
			if (str[0] == '#') {
				continue;
			}
			size_t n = str.find_first_of('=');
			if (n != std::string::npos) {
				std::string keyStr = str.substr(0, n);
				std::string valueStr = str.substr(n + 1);
				kvList_.emplace(keyStr, valueStr);
				result = true;
			}
		}
	}
	cfgFile.close();
	return result;
}

// ��ȡ�����ļ�
char * readItem(KVStringPair kvList_, const char * pItem_)
{
	if (!kvList_.empty()) {
		if (pItem_ && strlen(pItem_)) {
			KVStringPair::iterator iter = kvList_.find(pItem_);
			if (iter != kvList_.end()) {
				std::string strValue = iter->second;
				size_t nSize = strValue.size();
				if (nSize) {
					char * value = new char[nSize + 1];
					memset(value, 0, nSize + 1);
					if (value) {
						strcpy_s(value, nSize + 1, strValue.c_str());
						value[nSize] = '\0';
						return value;
					}
				}
			}
		}
	}
	return NULL;
}

// ����Զ�̷������ļ���Ŀ¼
BOOL FtpSession::SetServerPath(char* strServerPath)
{
	if (NULL == m_pFtpConn
		&& !ConnectToServer())
		return FALSE;

	return m_pFtpConn->SetCurrentDirectory(strServerPath);

}


// ��ʼ��log��־
void initLog(const char * pLogPath_)
{
	m_ullLogInst = LOG_Init();
	if (m_ullLogInst) {
		pf_logger::LogConfig logConf;
		strcpy_s(logConf.szLogPath, sizeof(logConf.szLogPath), pLogPath_);
		logConf.usLogPriority = pf_logger::eLOGPRIO_ALL;
		logConf.usLogType = pf_logger::eLOGTYPE_FILE;
		LOG_SetConfig(m_ullLogInst, logConf);
	}
}

// ����ʱ���
__int64 TimeDiff(SYSTEMTIME t1, SYSTEMTIME t2)
{
	CTimeSpan			sp;
	int					s1, s2;

	CTime tm1(t1.wYear, t1.wMonth, t1.wDay, 0, 0, 0);
	CTime tm2(t2.wYear, t2.wMonth, t2.wDay, 0, 0, 0);

	sp = tm1 - tm2;

	s1 = t1.wHour * 3600 + t1.wMinute * 60 + t1.wSecond;
	s2 = t2.wHour * 3600 + t2.wMinute * 60 + t2.wSecond;

	return  sp.GetDays() * 86400 + (s2 - s1);
}

// ��ʽ���ļ���С
void __fastcall FormatSize(const int ByteSize, char * Res)
{
	int i = ByteSize;
	if (i < 1024) {
		sprintf_s(Res, 128, "%d B", i);
		return;
	}
	if (i < 1024 * 1024) {
		sprintf_s(Res, 128, "%.2f KB", ((double)i) / (double)1024.0);
		return;
	}
	if (i < 1024 * 1024 * 1024) {
		sprintf_s(Res, 128, "%.2f MB", ((double)i) / (double)1024.0 / (double)1024.0);
		return;
	}
	else {
		sprintf_s(Res, 128, "%.2f GB", ((double)i) / (double)1024.0 / (double)1024.0 / (double)1024.0);
		return;
	}
}

int _tmain(int argc, _TCHAR* argv[]) {

	// ��ȡ�����ļ�
	char szCfgFileName[256] = { 0 };
	char szExePath[256] = { 0 };
	GetModuleFileNameA(NULL, szExePath, sizeof(szExePath));
	char szDrive[32] = { 0 };
	char szDir[256] = { 0 };
	_splitpath_s(szExePath, szDrive, sizeof(szDrive), szDir, sizeof(szDir), NULL, 0, NULL, 0);
	sprintf_s(szCfgFileName, sizeof(szCfgFileName), "%s%sconf\\ftp.data", szDrive, szDir);

	char szFtpIp[32] = { 0 };				//ip��ַ
	char szFtpUsername[32] = { 0 };			//�˺�
	char szFtpPassword[64] = { 0 };			//����
	char szLocalPath[64] = { 0 };			//����·��
	char szServerPath[64] = { 0 };			//������·��
	char szLog[512] = { 0 };
	unsigned short usFtpPort = 21;
	unsigned short usInterval = 30;
	KVStringPair kvPair;

	// ����������
	if (loadCfg(szCfgFileName, kvPair)) {
		char * ftp_ip = readItem(kvPair, "ftp_ip");
		if (ftp_ip) {
			strcpy_s(szFtpIp, sizeof(szFtpIp), ftp_ip);
			delete ftp_ip;
			ftp_ip = NULL;
		}
		char * ftp_port = readItem(kvPair, "ftp_port");
		if (ftp_port) {
			usFtpPort = (unsigned short)strtol(ftp_port, NULL, 10);
			delete ftp_port;
			ftp_port = NULL;
		}
		char * ftp_username = readItem(kvPair, "ftp_username");
		if (ftp_username) {
			strcpy_s(szFtpUsername, sizeof(szFtpUsername), ftp_username);
			delete ftp_username;
			ftp_username = NULL;
		}
		char * ftp_password = readItem(kvPair, "ftp_password");
		if (ftp_password) {
			strcpy_s(szFtpPassword, sizeof(szFtpPassword), ftp_password);
			delete ftp_password;
			ftp_password = NULL;
		}
		char * ftp_localdir = readItem(kvPair, "local_dir");
		if (ftp_localdir) {
			strcpy_s(szLocalPath, sizeof(szLocalPath), ftp_localdir);
			delete ftp_localdir;
			ftp_localdir = NULL;
		}
		char * ftp_serverdir = readItem(kvPair, "server_dir");
		if (ftp_serverdir) {
			strcpy_s(szServerPath, sizeof(szServerPath), ftp_serverdir);
			delete ftp_serverdir;
			ftp_serverdir = NULL;
		}
		char * ftp_interval = readItem(kvPair, "interval");
		if (ftp_interval) {
			usInterval = (unsigned short)strtol(ftp_interval, NULL, 10);
			delete ftp_interval;
			ftp_interval = NULL;
		}
	}

	char szPath[256] = { 0 };
	if (szLocalPath) {
		sprintf_s(szPath, sizeof(szPath), "%slog\\", szLocalPath);
		CreateDirectoryExA(".\\", szPath, NULL);
	}
	else {
		GetDllDirectoryA(sizeof(szPath), szPath);
		strcat_s(szPath, sizeof(szPath), "log\\");
		CreateDirectoryExA(".\\", szPath, NULL);
	}
	strcat_s(szPath, sizeof(szPath), "ftpDemo\\");
	m_usLogType = pf_logger::eLOGTYPE_FILE;

	// ��ʼ����־ϵͳ
	initLog(szPath);

	// ��ʼ������ftp������
	FtpSession ftp;
	ftp.SetServerParam(szFtpIp, usFtpPort);
	ftp.SetUserInfo(szFtpUsername, szFtpPassword, FALSE);
	bool conn = ftp.ConnectToServer();
	
	if (conn == false) {
		memset(szLog, 0, sizeof(szLog) / sizeof(char));
		sprintf_s(szLog, sizeof(szLog), "[ftpDemo]connect to server failed! "
			"error code = %s\n", ftp.GetLastErrorMessage());
		LOG_Log(m_ullLogInst, szLog, pf_logger::eLOGCATEGORY_INFO, m_usLogType);
		return 0;
	}

	cout << "input D/U to choose download or upload files" << endl;
	char flag;
	cin >> flag;
	
	switch (flag) {
	case 'D':
	case 'd': {
		while (1) {
			cout << "ftp download loop begin!" << endl;

			// ��ȡ�ļ��б�
			std::vector<FileProperty> fileList;
			ftp.GetFileList(szServerPath, fileList);

			if (0 == fileList.empty()) {
				for (std::vector<FileProperty>::const_iterator iter = fileList.begin(); iter != fileList.end(); iter++) {
					char beginTime[128] = { 0 };
					char endTime[128] = { 0 };
					char fileSize[128] = { 0 };
					__int64 costTime = 0;
					SYSTEMTIME beginSys;
					SYSTEMTIME endSys;
					ULONGLONG ulSize;
					CFileStatus fileStatus;

					// ��ȡ��ʼʱ��
					GetLocalTime(&beginSys);

					sprintf_s(beginTime, sizeof(beginTime), "%4d/%02d/%02d %02d:%02d:%02d.%03d", beginSys.wYear,
						beginSys.wMonth, beginSys.wDay, beginSys.wHour, beginSys.wMinute, beginSys.wSecond, beginSys.wMilliseconds);

					/*
					// ��ȡ��ǰ����Ŀ¼
					char szDir1[MAX_PATH] = { 0 };
					DWORD dwLen1 = GetCurrentDirectoryA(MAX_PATH, szDir1);
					cout << "the current path = " << szDir1 << endl;
					*/

					int nValidSetPath = ftp.SetServerPath(szServerPath);
					if (nValidSetPath == 0) {
						memset(szLog, 0, sizeof(szLog) / sizeof(char));
						sprintf_s(szLog, sizeof(szLog), "[ftpDemo]set path failed! filename:%s, size:%u, begin time:%s, "
							"error code:%s\n", iter->strFileName, ulSize, beginTime, ftp.GetLastErrorMessage());
						LOG_Log(m_ullLogInst, szLog, pf_logger::eLOGCATEGORY_INFO, m_usLogType);
					}

					// ƴ�ӱ�������·��
					CString localfilepath;
					localfilepath.Format("%s%s", szLocalPath, iter->strFileName);
					// �����ļ�
					int nValidGetFile = ftp.GetFileFromServer(iter->strFileName, localfilepath, true);
					if (nValidGetFile == 0) {
						memset(szLog, 0, sizeof(szLog) / sizeof(char));
						sprintf_s(szLog, sizeof(szLog), "[ftpDemo]download failed! filename:%s, size:%u, begin time:%s, "
							"error code:%s\n", iter->strFileName, ulSize, beginTime, ftp.GetLastErrorMessage());
						LOG_Log(m_ullLogInst, szLog, pf_logger::eLOGCATEGORY_INFO, m_usLogType);
					}

					// ��ȡ�ļ���С
					if (CFile::GetStatus(localfilepath, fileStatus)) {
						ulSize = fileStatus.m_size;
						FormatSize(ulSize, fileSize);
					}

					// ���سɹ���ɾ���������ļ�
					if (nValidGetFile != 0) {
						int bValidRemove = ftp.RemoveFileFromServer(iter->strFileName);
						if (bValidRemove == 0) {
							memset(szLog, 0, sizeof(szLog) / sizeof(char));
							sprintf_s(szLog, sizeof(szLog), "[ftpDemo]delete failed! filename:%s, size:%u, begin time:%s, "
								"end time:%s, total time:%d, error code:%s\n", iter->strFileName, ulSize, beginTime, endTime,
								costTime, ftp.GetLastErrorMessage());
							LOG_Log(m_ullLogInst, szLog, pf_logger::eLOGCATEGORY_INFO, m_usLogType);
							cout << "delete file failed! error code = " << ftp.GetLastErrorMessage() << endl;

						}
					}
					// ��ȡ����ʱ��
					GetLocalTime(&endSys);

					sprintf_s(endTime, sizeof(endTime), "%4d/%02d/%02d %02d:%02d:%02d.%03d", endSys.wYear,
						endSys.wMonth, endSys.wDay, endSys.wHour, endSys.wMinute, endSys.wSecond, endSys.wMilliseconds);

					costTime = TimeDiff(beginSys, endSys);

					memset(szLog, 0, sizeof(szLog) / sizeof(char));
					sprintf_s(szLog, sizeof(szLog), "[ftpDemo]download finish! filename:%s, size:%s, begin time:%s, "
						"end time:%s, total time:%d\n", iter->strFileName, fileSize, beginTime, endTime, costTime);
					LOG_Log(m_ullLogInst, szLog, pf_logger::eLOGCATEGORY_INFO, m_usLogType);
				}
			}
			//ÿinterval��ɨ��һ���ļ����������ز�ɾ��
			Sleep(usInterval * 1000);
		}
		break;
	}
	case 'U':
	case 'u': {
		while (1) {
			cout << "ftp upload loop begin!" << endl;

			// ��ȡ�ļ��б�
			std::vector<FileProperty> fileList;
			ftp.GetFileFromDirectory(szLocalPath, fileList);
			if (0 == fileList.empty()) {
				for (std::vector<FileProperty>::const_iterator iter = fileList.begin(); iter != fileList.end(); iter++) {
					char beginTime[128] = { 0 };
					char endTime[128] = { 0 };
					char fileSize[128] = { 0 };
					__int64 costTime = 0;
					SYSTEMTIME beginSys;
					SYSTEMTIME endSys;
					ULONGLONG ulSize;
					CFileStatus fileStatus;
					CString localfilepath;

					// ��ȡ��ʼʱ��
					GetLocalTime(&beginSys);

					sprintf_s(beginTime, sizeof(beginTime), "%4d/%02d/%02d %02d:%02d:%02d.%03d", beginSys.wYear,
						beginSys.wMonth, beginSys.wDay, beginSys.wHour, beginSys.wMinute, beginSys.wSecond, beginSys.wMilliseconds);

					// ����Զ�̷�����ָ��Ŀ¼
					
					int nValidSetPath = ftp.SetServerPath(szServerPath);
					if (nValidSetPath == 0) {
						memset(szLog, 0, sizeof(szLog) / sizeof(char));
						sprintf_s(szLog, sizeof(szLog), "[ftpDemo]set path failed! filename:%s, size:%u, begin time:%s, "
							"error code:%s\n", iter->strFileName, ulSize, beginTime, ftp.GetLastErrorMessage());
						LOG_Log(m_ullLogInst, szLog, pf_logger::eLOGCATEGORY_INFO, m_usLogType);
					}

					/*
					// ��ȡ��ǰ����Ŀ¼
					char szDir1[MAX_PATH] = { 0 };
					DWORD dwLen1 = GetCurrentDirectoryA(MAX_PATH, szDir1);
					cout << "the current path = " << szDir1 << endl;
					*/

					// ƴ��ftp�ϴ�·��
					if (PathFileExists(szLocalPath)) {
						localfilepath.Format("%s%s", szLocalPath, iter->strFileName);
					}
					else {
						return 0;
					}
					
					// �ϴ��ļ�
					bool nValidUpload = ftp.PutFileToServer(localfilepath, iter->strFileName);
					if (nValidUpload == 0) {
						memset(szLog, 0, sizeof(szLog) / sizeof(char));
						sprintf_s(szLog, sizeof(szLog), "[ftpDemo]upload failed! filename:%s, size:%u, begin time:%s, "
							"error code:%s\n", iter->strFileName, ulSize, beginTime, ftp.GetLastErrorMessage());
						LOG_Log(m_ullLogInst, szLog, pf_logger::eLOGCATEGORY_INFO, m_usLogType);
					}

					// ��ȡ�ļ���С
					if (CFile::GetStatus(localfilepath, fileStatus)) {
						ulSize = fileStatus.m_size;
						FormatSize(ulSize, fileSize);
					}

					// �ϴ��ɹ���ɾ�������ļ�
					if (nValidUpload != 0) {
						bool nValidRemove = ftp.RemoveFileFromLocal(localfilepath);
						if (nValidRemove == 0) {
							memset(szLog, 0, sizeof(szLog) / sizeof(char));
							sprintf_s(szLog, sizeof(szLog), "[ftpDemo]delete failed! filename:%s, size:%u, begin time:%s, "
								"end time:%s, total time:%d, error code:%s\n", iter->strFileName, ulSize, beginTime, endTime,
								costTime, ftp.GetLastErrorMessage());
							LOG_Log(m_ullLogInst, szLog, pf_logger::eLOGCATEGORY_INFO, m_usLogType);
							cout << "delete file failed! error code = " << ftp.GetLastErrorMessage() << endl;

						}
					}
					// ��ȡ����ʱ��
					GetLocalTime(&endSys);

					sprintf_s(endTime, sizeof(endTime), "%4d/%02d/%02d %02d:%02d:%02d.%03d", endSys.wYear,
						endSys.wMonth, endSys.wDay, endSys.wHour, endSys.wMinute, endSys.wSecond, endSys.wMilliseconds);

					costTime = TimeDiff(beginSys, endSys);

					memset(szLog, 0, sizeof(szLog) / sizeof(char));
					sprintf_s(szLog, sizeof(szLog), "[ftpDemo]upload finish! filename:%s, size:%s, begin time:%s, "
						"end time:%s, total time:%d\n", iter->strFileName, fileSize, beginTime, endTime, costTime);
					LOG_Log(m_ullLogInst, szLog, pf_logger::eLOGCATEGORY_INFO, m_usLogType);
				}
			}
			//ÿinterval��ɨ��һ���ļ��������ϴ���ɾ��
			Sleep(usInterval * 1000);
		}
		break;
	}
	}

	
	ftp.CloseConnection();
	return 0;
}
