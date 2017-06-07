#pragma once
// cpx-generator.cpp : specific code to be included in cpx generator scripts


namespace cpx {

    string
Work_dir()
{
    char const* include_path_name_cstr= getenv("CPX_WORK_DIR");
    if ( Is_null( include_path_name_cstr)) {
        ThrowBreak("environment variable CPX_WORK_DIR is not defined. "
                "NOTE: Script that is calling cpx::Work_dir() is intended "
                "to be called within the CPX system to generate code."
                ,eBC_handled
                );
    }
    return include_path_name_cstr;
}


} // namespace cpx
