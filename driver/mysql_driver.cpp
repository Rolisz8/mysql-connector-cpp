/*
   Copyright (c) 2008, 2010, Oracle and/or its affiliates. All rights reserved.

   The MySQL Connector/C++ is licensed under the terms of the GPL
   <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
   MySQL Connectors. There are special exceptions to the terms and
   conditions of the GPL as it is applied to this software, see the
   FLOSS License Exception
   <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
*/

#include "mysql_connection.h"

#include <cppconn/exception.h>

// Looks like this one should go after private_iface
#include "mysql_driver.h"
#include "nativeapi/native_driver_wrapper.h"


extern "C"
{
CPPCONN_PUBLIC_FUNC void * sql_mysql_get_driver_instance()
{
	void * ret = sql::mysql::get_driver_instance();
	return ret;
}


/* these are the functions without namespace - from cppconn/driver.h */
CPPCONN_PUBLIC_FUNC sql::Driver * get_driver_instance_by_name(const char * const clientlib)
{
	return sql::mysql::get_driver_instance_by_name(clientlib);
}


CPPCONN_PUBLIC_FUNC sql::Driver * get_driver_instance()
{
	return sql::mysql::get_driver_instance();
}


} /* extern "C" */

namespace sql
{
namespace mysql
{

static const ::sql::SQLString emptyStr("");
/* Mapping by client name is probably not enough here */
static std::map< sql::SQLString, boost::shared_ptr<MySQL_Driver> > driver;

CPPCONN_PUBLIC_FUNC sql::mysql::MySQL_Driver * get_driver_instance()
{
	return get_driver_instance_by_name("");
}


CPPCONN_PUBLIC_FUNC sql::mysql::MySQL_Driver * get_driver_instance_by_name(const char * const clientlib)
{
	::sql::SQLString dummy(clientlib);

	std::map< sql::SQLString, boost::shared_ptr< MySQL_Driver > >::const_iterator cit;

	if ((cit = driver.find(dummy)) != driver.end()) {
		return cit->second.get();
	} else {
		boost::shared_ptr< MySQL_Driver > newDriver;

		newDriver.reset(new MySQL_Driver(dummy));
		driver[dummy] = newDriver;

		return newDriver.get();
	}
}


MySQL_Driver::MySQL_Driver()
{
	try {
		proxy.reset(::sql::mysql::NativeAPI::createNativeDriverWrapper(emptyStr));
	}	catch(std::runtime_error & e)	{
		throw sql::InvalidArgumentException(e.what());
	}
}


MySQL_Driver::MySQL_Driver(const ::sql::SQLString & clientLib)
{
	try {
		proxy.reset(::sql::mysql::NativeAPI::createNativeDriverWrapper(clientLib));
	}	catch(std::runtime_error & e)	{
		throw sql::InvalidArgumentException(e.what());
	}
}


MySQL_Driver::~MySQL_Driver()
{
}


sql::Connection * MySQL_Driver::connect(const sql::SQLString& hostName,
										const sql::SQLString& userName,
										const sql::SQLString& password)
{
	return new MySQL_Connection(this, proxy->conn_init(),hostName, userName, password);
}


sql::Connection * MySQL_Driver::connect(sql::ConnectOptionsMap & properties)
{
	return new MySQL_Connection(this, proxy->conn_init(),properties);
}


int MySQL_Driver::getMajorVersion()
{
	return 1;
}

int MySQL_Driver::getMinorVersion()
{
	return 0;
}

int MySQL_Driver::getPatchVersion()
{
	return 6;
}

const sql::SQLString & MySQL_Driver::getName()
{
	static const sql::SQLString name("MySQL Connector C++ (libmysql)");
	return name;
}


void MySQL_Driver::threadInit()
{
	proxy->thread_init();
}


void MySQL_Driver::threadEnd()
{
	proxy->thread_end();
}

} /* namespace mysql */
} /* namespace sql */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
