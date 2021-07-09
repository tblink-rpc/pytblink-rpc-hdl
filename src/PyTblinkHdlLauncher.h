/*
 * PyTblinkHdlLauncher.h
 *
 *  Created on: Jul 8, 2021
 *      Author: mballance
 */

#include "ILaunch.h"
#include "tblink_rpc/IEndpointServices.h"

class PyTblinkHdlLauncher : public virtual ILaunch {
public:
	PyTblinkHdlLauncher();

	virtual ~PyTblinkHdlLauncher();

	virtual IEndpoint *launch(
			IEndpointServices		*services
			) override;

	virtual std::string last_error() override;

private:

	std::string find_python();

};

