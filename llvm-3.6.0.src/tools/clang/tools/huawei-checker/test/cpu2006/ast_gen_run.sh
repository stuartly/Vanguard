find . -name "ast_gen.sh" -execdir bash -c "pwd && mkdir ast" \;
find . -name "ast_gen.sh" -execdir bash -c "pwd && chmod +x ast_gen.sh && ./ast_gen.sh" \;
