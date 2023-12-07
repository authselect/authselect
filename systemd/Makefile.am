systemdunitdir=@SYSTEMD_UNIT_DIR@

dist_systemdunit_DATA = \
	authselect.service \
	$(NULL)

# Hook this into sysinit.target by default
install-exec-hook:
	$(MKDIR_P) $(DESTDIR)/$(systemdunitdir)/sysinit.target.wants
	$(LN_S) -f ../authselect.service $(DESTDIR)/$(systemdunitdir)/sysinit.target.wants/authselect.service
