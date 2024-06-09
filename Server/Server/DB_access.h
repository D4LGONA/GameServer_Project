#pragma once
#include "stdafx.h"

void DB_error(SQLHANDLE handle, SQLSMALLINT handleType, SQLRETURN retCode) 
{
    if (retCode == SQL_INVALID_HANDLE) 
    {
        fwprintf(stderr, L"Invalid handle!\n");
        return;
    }

    SQLSMALLINT recordNumber = 0;
    SQLINTEGER errorCode;
    WCHAR errorMessage[1000];
    WCHAR sqlState[SQL_SQLSTATE_SIZE + 1];

    while (SQLGetDiagRec(handleType, handle, ++recordNumber, sqlState, &errorCode, errorMessage,
        sizeof(errorMessage) / sizeof(WCHAR), nullptr) == SQL_SUCCESS) 
    {
        if (wcsncmp(sqlState, L"01004", 5) != 0) 
            fwprintf(stderr, L"[%5.5s] %s (%d)\n", sqlState, errorMessage, errorCode);
    }
}

bool DB_connect(SQLHDBC& hdbc, SQLHSTMT& hstmt) 
{
    SQLHENV henv = SQL_NULL_HENV; // ���� ������ ȯ�� �ڵ� ����
    SQLRETURN ret;

    // ȯ�� �ڵ� �Ҵ�
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        DB_error(SQL_NULL_HANDLE, SQL_HANDLE_ENV, ret);
        return false;
    }

    // ODBC ���� ����
    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        DB_error(henv, SQL_HANDLE_ENV, ret);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    // �����ͺ��̽� �ڵ� �Ҵ�
    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        DB_error(henv, SQL_HANDLE_DBC, ret);
        SQLFreeHandle(SQL_HANDLE_ENV, henv); // ȯ�� �ڵ� ����
        return false;
    }

    // �����ͺ��̽� ����
    ret = SQLConnect(hdbc, (SQLWCHAR*)L"2022184015_GS_Project", SQL_NTS, NULL, 0, NULL, 0);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
    {
        ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) // ������ ���
        {
            DB_error(hdbc, SQL_HANDLE_DBC, ret);
            SQLDisconnect(hdbc);
            SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
            SQLFreeHandle(SQL_HANDLE_ENV, henv); // ȯ�� �ڵ� ����
            return false;
        }
        return true;
    }
    else
    {
        DB_error(hdbc, SQL_HANDLE_DBC, ret);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv); // ȯ�� �ڵ� ����
        hdbc = SQL_NULL_HDBC;
        return false;
    }
}

bool DB_disconnect(SQLHDBC& hdbc, SQLHSTMT& hstmt)
{
    bool success = true;

    if (hstmt != SQL_NULL_HSTMT)
    {
        if (SQLFreeHandle(SQL_HANDLE_STMT, hstmt) != SQL_SUCCESS)
            success = false;
        hstmt = SQL_NULL_HSTMT;
    }

    if (hdbc != SQL_NULL_HDBC)
    {
        if (SQLDisconnect(hdbc) != SQL_SUCCESS)
            success = false;
        if (SQLFreeHandle(SQL_HANDLE_DBC, hdbc) != SQL_SUCCESS)
            success = false;
        hdbc = SQL_NULL_HDBC;
    }

    return success;
}
