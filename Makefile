BOB_VERSION := 1.9.0
BOB_FILE := /tmp/bob.$(BOB_VERSION).jar
$(BOB_FILE):
	@echo "Download bob.jar"
	@wget 'https://github.com/defold/defold/releases/download/$(BOB_VERSION)/bob.jar' -O $(BOB_FILE)

buildweb: $(BOB_FILE)
	@echo "Build html5 (web)"
	@java -jar $(BOB_FILE) \
		--settings game.project \
		--texture-compression true \
		--bundle-output dist \
		--variant debug \
		--archive \
		--platform js-web \
		--use-async-build-server \
		resolve clean distclean build bundle

serve3:
	@echo "Serve dist directory on http://localhost:8000"
	@cd dist/def_exprtk_lua && python3 -m http.server 8000
