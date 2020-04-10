#include <afxinet.h>
#include <vector>

#define FTP_DEFAULT_PORT 21

struct FileProperty
{
	CString strFileName;
	// FILE_ATTRIBUTE_DIRECTORY 
	// GetFileAttributes
	DWORD dwFileType;
};

class FtpSession
{
public:
	FtpSession(void);
	~FtpSession(void);

	// ����FTP��������ַ���˿�
	void SetServerParam(CString strServerAddr, INTERNET_PORT wServerPort);
	// ���õ�¼��������
	void SetUserInfo(CString strUserName, CString strPassWord, BOOL bInitiativeMode = FALSE);
	// ���ӵ�ָ��FTP������
	BOOL ConnectToServer();
	// �õ�ָ��Ŀ¼�µ������ļ�
	void GetFileList(CString strDirPath, std::vector<FileProperty> &vctFileList);
	// �ϴ�һ���ļ���FTP������
	BOOL PutFileToServer(CString strLocalFile, CString strPutPath);
	// ��FTP����������һ���ļ�
	BOOL GetFileFromServer(CString strRemoteFile, CString strLocalPath, BOOL bFailIfExists = FALSE);
	// ���ñ��������ļ�·��
	BOOL SetLocalFilePath(char* strLocalFilePath);
	// ��FTP������ɾ��һ���ļ�
	BOOL RemoveFileFromServer(CString strRemoteFile);
	// �ر�����
	void CloseConnection();
	//��ȡ�����ļ����ļ��б�
	void GetFileFromDirectory(CString csDirPath, std::vector<FileProperty> &vctPath);
	// ɾ�������ļ�
	BOOL RemoveFileFromLocal(CString strRemoteFile);
	// �������Ƿ����
	BOOL IsConnectionActive();
	// ����Զ�̷������ļ���Ŀ¼
	BOOL SetServerPath(char* strServerPath);

	// ��ȡ���һ��������Ϣ
	CString GetLastErrorMessage();

private:
	// FTP��������ַ
	CString m_strServerAddr;
	// ����˿�
	INTERNET_PORT m_wServerPort;
	// ��¼�û���
	CString m_strUserName;
	// ��¼����
	CString m_strPassWord;
	// ����ģʽ-FALSE  ����ģʽ-TRUE
	BOOL m_bInitiativeMode;
	// ��¼���һ��������Ϣ
	CString m_strLastErrorMsg;

	// ��������ʼ��һ������ͬʱ��Internet �Ự
	CInternetSession m_cInetSession;
	// ������Internet��������FTP���Ӳ�����ֱ�Ӳ��ݷ������е�Ŀ¼���ļ�
	CFtpConnection *m_pFtpConn;
};