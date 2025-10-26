#!/usr/local/bin/cpx




#!
INFO("Test ScopedDIrectory",VARVALS(fs::current_path()));
{
    ScopedDirectory scp1("h/Documents");
    INFO(VARVALS(fs::current_path()));
    {
        ScopedDirectory scp2("schaak");
        INFO(VARVALS(fs::current_path()));
    }
    INFO(VARVALS(fs::current_path()));
}
INFO(VARVALS(fs::current_path()),ENDL);
INFO("Test DirectoryStack",VARVALS(fs::current_path()));
{
    DirectoryStack ds;
    ds.changeDir("h/Documents");
    INFO(VARVALS(fs::current_path()));
    ds.pushDir("schaak");
    INFO(VARVALS(fs::current_path()));
    INFO(VARVALS(ds));
    ds.popDir();
    INFO(VARVALS(fs::current_path()));
    ds.popDir();
    INFO(VARVALS(fs::current_path()));
}
