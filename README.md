# Create Your Profile - Frontend

Responsive static HTML/CSS implementation of the "Create Your Profile" screen.

## Tech
- HTML5 semantic structure
- CSS (Flexbox/Grid, mobile-first), no JavaScript
- Inter font from Google Fonts

## Run locally
Open `index.html` directly in your browser, or serve the folder with any static server.

Local preview using Python (already available on most systems):
```bash
cd /workspace
python3 -m http.server 5173
# then open http://localhost:5173 in your browser
```

## Deploy options
- GitHub Pages: Push this folder to a repo and enable Pages for the `main` branch.
- Netlify: Drag-and-drop this folder onto the Netlify dashboard, or connect the repo.
- Vercel: Import the repo, framework preset: "Other", output: `/`.

## Notes
- Desktop breakpoint: `min-width: 1024px`.
- Mobile layout stacks fields; desktop shows paired fields side-by-side.
- Colors, spacing, typography applied per brief: form background `#2B414F`, labels `#C6B98B`, inputs and placeholders `#999999`, primary button `#295BFF` with white text.

# create-profile-frontend