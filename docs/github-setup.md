# GitHub Setup Guide

This guide will help you create a GitHub repository for this project.

## Prerequisites

- GitHub account (create one at https://github.com if you don't have one)
- `gh` CLI tool (GitHub's command-line tool)

## Option 1: Using GitHub CLI (Recommended - Easiest)

### Step 1: Install GitHub CLI

```bash
brew install gh
```

### Step 2: Authenticate with GitHub

```bash
gh auth login
```

Follow the prompts:
- Choose "GitHub.com"
- Choose "HTTPS"
- Authenticate with your web browser (recommended)

### Step 3: Create the repository

From your project directory:

```bash
gh repo create boost-safeprofile --public --source=. --remote=origin --push
```

This will:
- Create a new public repository called "boost-safeprofile" on GitHub
- Set it as the "origin" remote
- Push your existing commits

**Done!** Your repository is now on GitHub.

### Verify

```bash
gh repo view --web
```

This opens your new repository in your browser.

## Option 2: Using the GitHub Website

### Step 1: Create repository on GitHub

1. Go to https://github.com/new
2. Repository name: `boost-safeprofile`
3. Description: "C++ Safety Profile conformance analysis tool"
4. Choose "Public" (or "Private" if you prefer)
5. **DO NOT** initialize with README, .gitignore, or license (we already have these)
6. Click "Create repository"

### Step 2: Connect your local repository

GitHub will show you instructions. Use these commands:

```bash
git remote add origin https://github.com/YOUR_USERNAME/boost-safeprofile.git
git branch -M main
git push -u origin main
```

Replace `YOUR_USERNAME` with your GitHub username.

## Verify Everything Worked

After setup, verify with:

```bash
# Check remote is configured
git remote -v

# Should show:
# origin  https://github.com/YOUR_USERNAME/boost-safeprofile.git (fetch)
# origin  https://github.com/YOUR_USERNAME/boost-safeprofile.git (push)

# Check your commits are on GitHub
gh repo view --web
# or visit: https://github.com/YOUR_USERNAME/boost-safeprofile
```

## Common Issues

### Authentication Failed

If you get authentication errors:

```bash
# Re-authenticate with GitHub CLI
gh auth login

# Or set up SSH keys (more advanced)
# See: https://docs.github.com/en/authentication/connecting-to-github-with-ssh
```

### Remote Already Exists

If you see "remote origin already exists":

```bash
# View existing remotes
git remote -v

# Remove old remote
git remote remove origin

# Try again
```

## Next Steps

After your repository is on GitHub:

1. **Add a repository description** on GitHub (via Settings)
2. **Add topics**: `cpp`, `safety`, `boost`, `static-analysis`, `security`
3. **Enable Issues** for bug tracking (usually enabled by default)
4. **Consider adding:**
   - Branch protection rules (Settings → Branches)
   - CI/CD workflows (we'll add these later)

## Making Changes and Pushing

After GitHub is set up, your workflow is:

```bash
# Make changes, then:
git add .
git commit -m "Describe your changes"
git push

# Pull changes from GitHub (if working from multiple machines):
git pull
```

## Getting Help

- GitHub CLI docs: https://cli.github.com/manual/
- Git basics: https://git-scm.com/book/en/v2/Getting-Started-About-Version-Control
- GitHub docs: https://docs.github.com
